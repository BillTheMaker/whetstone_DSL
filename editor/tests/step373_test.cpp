// Step 373: Common Lisp Integration (8 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "Pipeline.h"
#include "CrossLanguageProjector.h"
#include "AnnotationInference.h"
#include "CompactAST.h"
#include "SemannoSidecar.h"
#include "ast/CommonLispParser.h"
#include "ast/CommonLispGenerator.h"

static bool contains(const std::string& text, const std::string& token) {
    return text.find(token) != std::string::npos;
}

static bool hasInference(const std::vector<AnnotationInference::InferredAnnotation>& inferred,
                         const std::string& annotationType,
                         const std::string& value = "") {
    for (const auto& a : inferred) {
        if (a.annotationType != annotationType) continue;
        if (value.empty() || a.value == value) return true;
    }
    return false;
}

int main() {
    int passed = 0;

    // Test 1: Parse CL -> generate CL -> parse roundtrip shape
    {
        std::string src =
            "(defclass point () ((x) (y)))\n"
            "(defun add (a b) (+ a b))\n";
        auto mod1 = CommonLispParser::parseCommonLisp(src);
        CommonLispGenerator gen;
        std::string out = gen.generate(mod1.get());
        auto mod2 = CommonLispParser::parseCommonLisp(out);
        assert(mod1->getChildren("classes").size() == mod2->getChildren("classes").size());
        assert(mod1->getChildren("functions").size() == mod2->getChildren("functions").size());
        std::cout << "Test 1 PASSED: CL parse/generate/parse roundtrip\n";
        passed++;
    }

    // Test 2: Python -> Common Lisp (class -> CLOS)
    {
        Pipeline p;
        std::string py =
            "class Point:\n"
            "    def __init__(self, x, y):\n"
            "        self.x = x\n"
            "        self.y = y\n";
        auto res = p.run(py, "python", "common-lisp");
        assert(res.success);
        assert(contains(res.generatedCode, "(defclass"));
        std::cout << "Test 2 PASSED: Python class -> Common Lisp CLOS form\n";
        passed++;
    }

    // Test 3: Common Lisp -> Python projection retains class structure
    {
        auto src = CommonLispParser::parseCommonLisp("(defclass point () ((x) (y)))");
        CrossLanguageProjector projector;
        auto pyMod = projector.project(src.get(), "python");
        Pipeline p;
        auto code = p.generate(pyMod.get(), "python");
        assert(pyMod != nullptr);
        assert(pyMod->getChildren("classes").size() == 1);
        assert(!code.empty());
        std::cout << "Test 3 PASSED: Common Lisp class retained in Python projection\n";
        passed++;
    }

    // Test 4: Common Lisp -> Java projection retains class hierarchy shape
    {
        auto src = CommonLispParser::parseCommonLisp("(defclass rectangle (shape) ((w) (h)))");
        CrossLanguageProjector projector;
        auto javaMod = projector.project(src.get(), "java");
        assert(javaMod != nullptr);
        assert(javaMod->getChildren("classes").size() == 1);
        std::cout << "Test 4 PASSED: Common Lisp class retained in Java projection\n";
        passed++;
    }

    // Test 5: CL annotation inference (macro, dynamic var, tail call)
    {
        std::string cl =
            "(defmacro m (x) x)\n"
            "(defvar *state* 1)\n"
            "(defun loopf (n) (loopf n))\n";
        auto mod = CommonLispParser::parseCommonLisp(cl);
        AnnotationInference inf;
        auto inferred = inf.inferAll(mod.get());
        assert(hasInference(inferred, "MetaAnnotation", "quoted"));
        assert(hasInference(inferred, "BindingAnnotation", "dynamic"));
        assert(hasInference(inferred, "TailCallAnnotation"));
        std::cout << "Test 5 PASSED: CL annotation inference integration\n";
        passed++;
    }

    // Test 6: Semanno sidecar roundtrip on .lisp path
    {
        Module mod("m1", "cl_module", "common-lisp");
        auto* fn = new Function("f1", "run");
        fn->setSpan(1, 1, 1, 12);
        auto* owner = new OwnerAnnotation("a1", "Shared_GC");
        fn->addChild("annotations", owner);
        mod.addChild("functions", fn);

        std::string root = "/tmp/whetstone_step373";
        std::string filePath = "samples/test.lisp";
        auto save = saveSemannoSidecar(root, filePath, &mod);
        auto load = loadSemannoSidecar(root, filePath);
        assert(save.success);
        assert(load.success);
        assert(load.count >= 1);
        std::cout << "Test 6 PASSED: Semanno sidecar roundtrip for CL\n";
        passed++;
    }

    // Test 7: Pipeline.run with CL source succeeds
    {
        Pipeline p;
        std::string cl = "(defun id (x) x)";
        auto res = p.run(cl, "common-lisp", "common-lisp");
        assert(res.success);
        assert(!res.generatedCode.empty());
        std::cout << "Test 7 PASSED: Pipeline.run with CL source\n";
        passed++;
    }

    // Test 8: Compact AST has meaningful Lisp node names
    {
        auto mod = CommonLispParser::parseCommonLisp("(defun id (x) x)");
        auto compact = toJsonCompactTree(mod.get());
        assert(compact.is_array());
        bool hasFunctionNode = false;
        for (const auto& n : compact) {
            if (n.contains("type") && n["type"] == "Function") hasFunctionNode = true;
        }
        assert(hasFunctionNode);
        std::cout << "Test 8 PASSED: compact AST contains Function node\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
