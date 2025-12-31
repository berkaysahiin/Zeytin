module;

#include <string>
#include <vector>
#include <unordered_map>
#include <imgui.h>

export module zeytin.testviewer;

export enum class Status { 
    NONE = 0,
    TODO,
    PASSED,
    BLOCKED,
    SKIPPED,
    ACCEPTABLE,
    FAILED,
    LENGTH,
};

export struct TestStep {
    std::string id;
    std::string action;
    std::string data;
    std::string result;
    Status status = Status::TODO;
    std::string actual_result;
    std::string comment;
    std::vector<std::string> defects;
    std::vector<std::string> historical_defects;
};

export struct TestRun {
    std::string test_key;
    std::string test_summary;
    std::vector<TestStep> steps;
    Status status = Status::TODO;

    bool is_executed() const {
        if (status != Status::TODO) return true;
        for (const auto& step : steps) {
            if (step.status == Status::TODO || step.status == Status::NONE) return false;
        }
        return false;
    }
};

export struct TestExecution { 
    std::string key;
    std::string summary;
    std::string description;
    std::vector<TestRun> test_runs;

    int get_run_count_with_status(Status status) const {
        int count = 0;
        for(const auto& test_run : test_runs) {
            if(test_run.status == status) {
                count++;;
            }
        }
        return count;
    }

    int get_executed_test_runs_count() const {
        int count = 0;
        for(const auto& test_run : test_runs) {
            if(test_run.is_executed()) {
                count++;
            }
        }
        return count;
    }
};

export struct TestPlanOverview {
    std::string key;
    std::string summary;
    int test_count;
    std::vector<std::string> test_keys;
    std::vector<std::string> test_summaries;
};

export class TestViewer {
public:
    TestViewer();
    ~TestViewer();

    void render();
    void reset_all_tests();
    
private:
    void create_mockup_data();
    void render_plan_selection();
    void render_test_execution_view();
    void render_toolbar();
    void render_summary();
    void render_test_execution();
    void render_test_run(int index);
    void render_test_step(TestRun&, int step_idx);
    void load_tests_for_plan(const TestPlanOverview& plan);

    TestExecution m_test_execution;
    std::vector<TestPlanOverview> m_test_plans;
    bool m_plan_selected = false;
    int m_current_plan_index = 0;
    std::unordered_map<std::string, std::vector<char>> m_actual_result_buffers;
};
