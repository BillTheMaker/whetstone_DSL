// Step 4: Build SimpleFunctionExample from MPS with @deref(batched) annotation.
//
// Module "SimpleFunctionExample" (cpp)
// └── Function "calculate_sum"
//     ├── annotations: DerefStrategy(strategy: "batched")
//     ├── parameters: Parameter("a", int), Parameter("b", int)
//     ├── returnType: PrimitiveType("int")
//     └── body:
//         ├── Assignment { target: VarRef("result"), value: BinOp("+", VarRef("a"), VarRef("b")) }
//         └── Return { value: VarRef("result") }

#include "../src/ast/Module.h"
#include "../src/ast/Function.h"
#include "../src/ast/Variable.h"
#include "../src/ast/Parameter.h"
#include "../src/ast/Statement.h"
#include "../src/ast/Expression.h"
#include "../src/ast/Type.h"
#include "../src/ast/Annotation.h"
#include <cassert>
#include <iostream>

int main() {
    // --- Build the SimpleFunctionExample tree ---

    Module mod("SFE_M001", "SimpleFunctionExample", "cpp");

    Function calcSum("SFE_F001", "calculate_sum");

    // Annotation: @deref(batched)
    DerefStrategy deref("SFE_DR001", "batched");
    calcSum.addChild("annotations", &deref);

    // Parameters
    Parameter pa("SFE_P001", "a");
    PrimitiveType paType("SFE_PT001", "int");
    pa.setChild("type", &paType);
    calcSum.addChild("parameters", &pa);

    Parameter pb("SFE_P002", "b");
    PrimitiveType pbType("SFE_PT002", "int");
    pb.setChild("type", &pbType);
    calcSum.addChild("parameters", &pb);

    // Return type
    PrimitiveType retType("SFE_RT001", "int");
    calcSum.setChild("returnType", &retType);

    // Body: Assignment(result = a + b)
    Assignment assign;
    assign.id = "SFE_A001";
    VariableReference assignTarget("SFE_VR001", "result");
    assign.setChild("target", &assignTarget);

    BinaryOperation addOp("SFE_BO001", "+");
    VariableReference vrA("SFE_VR_a", "a");
    VariableReference vrB("SFE_VR_b", "b");
    addOp.setChild("left", &vrA);
    addOp.setChild("right", &vrB);
    assign.setChild("value", &addOp);

    calcSum.addChild("body", &assign);

    // Body: Return(result)
    Return ret;
    ret.id = "SFE_R001";
    VariableReference vrRet("SFE_VR_ret", "result");
    ret.setChild("value", &vrRet);
    calcSum.addChild("body", &ret);

    mod.addChild("functions", &calcSum);

    // --- Verify annotations ---

    auto* fn = static_cast<Function*>(mod.getChildren("functions")[0]);
    assert(fn->name == "calculate_sum");

    const auto& annots = fn->getChildren("annotations");
    assert(annots.size() == 1);
    assert(annots[0]->conceptType == "DerefStrategy");

    auto* ds = static_cast<DerefStrategy*>(annots[0]);
    assert(ds->strategy == "batched");
    assert(ds->parent == fn);
    assert(ds->id == "SFE_DR001");

    // Verify annotation sits alongside other children
    assert(fn->getChildren("parameters").size() == 2);
    assert(fn->getChildren("body").size() == 2);
    assert(fn->getChild("returnType") != nullptr);

    // Test OptimizationLock on Module
    OptimizationLock lock;
    lock.id = "OL001";
    lock.lockedBy = "human";
    lock.lockReason = "performance-critical path";
    lock.lockLevel = "full";
    lock.affectedStrategies = "batched,lazy";
    lock.timestamp = "2026-02-06T12:00:00Z";
    mod.addChild("annotations", &lock);

    const auto& modAnnots = mod.getChildren("annotations");
    assert(modAnnots.size() == 1);
    auto* ol = static_cast<OptimizationLock*>(modAnnots[0]);
    assert(ol->lockedBy == "human");
    assert(ol->lockReason == "performance-critical path");
    assert(ol->lockLevel == "full");
    assert(ol->parent == &mod);

    // Test LangSpecific on Variable
    Variable v("V001", "counter");
    mod.addChild("variables", &v);

    LangSpecific langSpec;
    langSpec.id = "LS001";
    langSpec.language = "cpp";
    langSpec.idiomType = "constexpr";
    langSpec.rawSyntax = "constexpr int counter = 0;";
    langSpec.semanticHint = "compile_time";
    langSpec.position = "before";
    v.addChild("annotations", &langSpec);

    const auto& varAnnots = v.getChildren("annotations");
    assert(varAnnots.size() == 1);
    auto* ls = static_cast<LangSpecific*>(varAnnots[0]);
    assert(ls->language == "cpp");
    assert(ls->idiomType == "constexpr");
    assert(ls->rawSyntax == "constexpr int counter = 0;");
    assert(ls->semanticHint == "compile_time");
    assert(ls->parent == &v);

    // DerefStrategy with derefTime child
    DerefStrategy timedDeref("DS002", "lazy");
    timedDeref.derefLocation = "call_site";
    timedDeref.owner = "runtime";
    IntegerLiteral derefTime("DT001", 500);
    timedDeref.setChild("derefTime", &derefTime);

    assert(timedDeref.getChild("derefTime") != nullptr);
    auto* dt = static_cast<IntegerLiteral*>(timedDeref.getChild("derefTime"));
    assert(dt->value == 500);
    assert(dt->parent == &timedDeref);

    std::cout << "Step 4: PASS — Annotations (DerefStrategy, OptimizationLock, LangSpecific)" << std::endl;
    return 0;
}
