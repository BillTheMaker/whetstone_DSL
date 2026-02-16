// Step 391: Workflow Templates (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "WorkflowTemplates.h"

int main() {
    int passed = 0;

    // Test 1: 5 templates available
    {
        auto templates = WorkflowTemplates::getTemplates();
        assert(templates.size() == 5);
        std::cout << "PASS: test 1 — 5 templates available\n";
        passed++;
    }

    // Test 2: CRUD template creates correct skeleton
    {
        TemplateParams params;
        params.entityName = "User";
        auto wf = WorkflowTemplates::applyTemplate("crud-api", params);

        assert(wf.queue.size() == 5);
        auto readItem = wf.queue.getItem("crud-read");
        assert(readItem.has_value());
        assert(readItem->nodeName == "getUser");
        assert(readItem->workerType == "template");

        auto deleteItem = wf.queue.getItem("crud-delete");
        assert(deleteItem.has_value());
        assert(deleteItem->reviewRequired == true);
        assert(deleteItem->priority == "critical");
        std::cout << "PASS: test 2 — CRUD template creates correct skeleton\n";
        passed++;
    }

    // Test 3: Cross-language template populates with annotation-guided routing
    {
        TemplateParams params;
        params.sourceLanguage = "python";
        params.targetLanguage = "rust";
        params.functionNames = {"getName", "processData", "setValue"};
        auto wf = WorkflowTemplates::applyTemplate("cross-language-port", params);

        assert(wf.queue.size() == 3);

        // getName is a getter → template worker
        auto getter = wf.queue.getItem("port-0");
        assert(getter.has_value());
        assert(getter->workerType == "template");

        // processData is complex → LLM
        auto complex = wf.queue.getItem("port-1");
        assert(complex.has_value());
        assert(complex->workerType == "llm");
        assert(complex->reviewRequired == true);
        std::cout << "PASS: test 3 — cross-language template routing\n";
        passed++;
    }

    // Test 4: Template params validated (unknown template returns empty)
    {
        TemplateParams params;
        auto wf = WorkflowTemplates::applyTemplate("nonexistent", params);
        assert(wf.queue.size() == 0);
        assert(!WorkflowTemplates::isValidTemplate("nonexistent"));
        assert(WorkflowTemplates::isValidTemplate("crud-api"));
        std::cout << "PASS: test 4 — template params validated\n";
        passed++;
    }

    // Test 5: Test generation template creates @Intent annotations (via naming)
    {
        TemplateParams params;
        params.functionNames = {"simpleAdd", "integrationAuth", "complexParse"};
        auto wf = WorkflowTemplates::applyTemplate("test-suite", params);

        assert(wf.queue.size() == 3);

        // Simple → SLM
        auto simple = wf.queue.getItem("test-0");
        assert(simple.has_value());
        assert(simple->workerType == "slm");

        // Integration → LLM
        auto integration = wf.queue.getItem("test-1");
        assert(integration.has_value());
        assert(integration->workerType == "llm");
        assert(integration->contextWidth == "project");

        // Complex → LLM
        auto complex = wf.queue.getItem("test-2");
        assert(complex.has_value());
        assert(complex->workerType == "llm");
        std::cout << "PASS: test 5 — test generation template\n";
        passed++;
    }

    // Test 6: applyTemplate produces valid WorkflowState
    {
        TemplateParams params;
        params.entityName = "Order";
        auto wf = WorkflowTemplates::applyTemplate("crud-api", params);

        auto stats = wf.getStats();
        assert(stats.total == 5);
        assert(stats.pending == 5 || stats.ready > 0); // items start pending
        assert(!wf.projectName.empty());
        std::cout << "PASS: test 6 — applyTemplate produces valid WorkflowState\n";
        passed++;
    }

    // Test 7: Module refactor template
    {
        TemplateParams params;
        params.functionNames = {"extractHelper", "renameVar", "splitClass"};
        auto wf = WorkflowTemplates::applyTemplate("module-refactor", params);

        assert(wf.queue.size() == 3);
        auto item = wf.queue.getItem("refactor-0");
        assert(item.has_value());
        assert(item->workerType == "llm");
        assert(item->reviewRequired == true);
        std::cout << "PASS: test 7 — module refactor template\n";
        passed++;
    }

    // Test 8: Template list includes descriptions
    {
        auto templates = WorkflowTemplates::getTemplates();
        for (const auto& t : templates) {
            assert(!t.name.empty());
            assert(!t.description.empty());
            assert(t.description.size() > 20);

            json j = t.toJson();
            assert(j.contains("name"));
            assert(j.contains("description"));
            assert(j.contains("requiredParams"));
        }
        std::cout << "PASS: test 8 — template list includes descriptions\n";
        passed++;
    }

    // Test 9: Empty params uses defaults
    {
        TemplateParams params;
        params.entityName = "Item";
        auto wf = WorkflowTemplates::applyTemplate("crud-api", params);
        // Default bufferId should be "main.py"
        auto item = wf.queue.getItem("crud-create");
        assert(item.has_value());
        assert(item->bufferId == "main.py");
        std::cout << "PASS: test 9 — empty params uses defaults\n";
        passed++;
    }

    // Test 10: Legacy modernization template
    {
        TemplateParams params;
        params.functionNames = {"oldFunc1", "oldFunc2", "oldFunc3"};
        auto wf = WorkflowTemplates::applyTemplate("legacy-modernization", params);

        assert(wf.queue.size() == 3);
        // First item (i=0) should be deterministic (i%3==0)
        auto first = wf.queue.getItem("modernize-0");
        assert(first.has_value());
        assert(first->workerType == "deterministic");

        // Second (i=1) should be llm
        auto second = wf.queue.getItem("modernize-1");
        assert(second.has_value());
        assert(second->workerType == "llm");
        std::cout << "PASS: test 10 — legacy modernization template\n";
        passed++;
    }

    // Test 11: CRUD template has dependency (update depends on create)
    {
        TemplateParams params;
        params.entityName = "Product";
        auto wf = WorkflowTemplates::applyTemplate("crud-api", params);

        auto update = wf.queue.getItem("crud-update");
        assert(update.has_value());
        assert(!update->dependencies.empty());
        assert(update->dependencies[0] == "crud-create");
        std::cout << "PASS: test 11 — CRUD template has dependency\n";
        passed++;
    }

    // Test 12: Default function count when names not given
    {
        TemplateParams params;
        params.functionCount = 3;
        auto wf = WorkflowTemplates::applyTemplate("module-refactor", params);
        assert(wf.queue.size() == 3);

        TemplateParams params2;
        params2.functionCount = 7;
        auto wf2 = WorkflowTemplates::applyTemplate("test-suite", params2);
        assert(wf2.queue.size() == 7);
        std::cout << "PASS: test 12 — default function count\n";
        passed++;
    }

    std::cout << "\nStep 391 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
