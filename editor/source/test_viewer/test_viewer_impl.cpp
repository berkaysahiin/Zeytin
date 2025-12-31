module;

#include "imgui.h"
#include <string>

module zeytin.testviewer;

static std::string get_status_string(Status status);
static ImVec4 get_status_color(Status status);
static ImVec4 get_status_bg_color(Status status);

TestViewer::TestViewer() {
    create_mockup_data();
}

TestViewer::~TestViewer() {}

void TestViewer::render() {
    if (!m_plan_selected) {
        render_plan_selection();
    } else {
        if (ImGui::Button("<- Back to Plan Selection")) {
            m_plan_selected = false;
        } else {
            render_test_execution_view();
        }
    }
}

void TestViewer::render_plan_selection() {
    ImGui::Text("Available Test Plans");
    ImGui::Separator();

    if (ImGui::BeginTable("test_plans_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Test Plan Key", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Summary", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Test Count", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < m_test_plans.size(); i++) {
            const auto& plan = m_test_plans[i];
            
            ImGui::PushID(i);
            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            if (ImGui::Selectable(plan.key.c_str(), false, ImGuiSelectableFlags_SpanAllColumns)) {
                m_plan_selected = true;
                m_current_plan_index = i;
                load_tests_for_plan(plan);
            }
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", plan.summary.c_str());
            
            ImGui::TableNextColumn();
            ImGui::Text("%d", plan.test_count);
            
            ImGui::PopID();
        }
        
        ImGui::EndTable();
    }
}

void TestViewer::render_test_execution_view() {
    render_toolbar();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text("Test Plan: %s", m_test_plans[m_current_plan_index].key.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "| %s", m_test_plans[m_current_plan_index].summary.c_str());
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginChild("test_summary", ImVec2(0, ImGui::GetTextLineHeight() * 4), true);
        render_summary();
    ImGui::EndChild();

    ImGui::Separator();
    
    ImGui::BeginChild("test_details", ImVec2(0, 0), true);
        render_test_execution();
    ImGui::EndChild();
}

void TestViewer::render_toolbar() {
    if (ImGui::Button("Save Results")) {}
    
    ImGui::SameLine();
    
    if (ImGui::Button("Reset All")) {
        reset_all_tests();
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Export to Jira")) {}
}

void TestViewer::render_summary() {
    float completion = m_test_execution.test_runs.empty() ? 0.0f : 
    (float)m_test_execution.get_executed_test_runs_count() / m_test_execution.test_runs.size();
    
    ImGui::Text("Progress:");
    ImGui::SameLine();
    
    float barWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 barPosition = ImGui::GetCursorPos();

    ImGui::ProgressBar(completion, ImVec2(barWidth, 14.0f), "");
    
    barPosition.x += barWidth * 0.5f - 20;
    ImGui::SetCursorPos(barPosition);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.0f%%", completion * 100);
    
    int column_count = static_cast<int>(Status::LENGTH);
    ImGui::Columns(column_count, "test_stats_columns", true);
    
    ImGui::Text("Total: %d", static_cast<int>(m_test_execution.test_runs.size()));
    ImGui::NextColumn();
    
    for (int i = 1; i < static_cast<int>(Status::LENGTH); i++) {
        const auto& status = static_cast<Status>(i);
        const auto& color = get_status_color(status);  

        ImGui::TextColored(color, "%s: %d", get_status_string(status).c_str(), m_test_execution.get_run_count_with_status(status));
        ImGui::NextColumn();
    }
}

void TestViewer::render_test_execution() {
    for (int i = 0; i < m_test_execution.test_runs.size(); i++) {
        render_test_run(i);
        
        if (i < m_test_execution.test_runs.size() - 1) {
            ImGui::Separator();
        }
    }
}

void TestViewer::render_test_run(int index) {
    TestRun& run = m_test_execution.test_runs[index];

    ImGui::PushID(index);

    ImVec2 header_start_pos = ImGui::GetCursorScreenPos();

    ImGui::PushStyleColor(ImGuiCol_Header, get_status_bg_color(run.status));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, get_status_bg_color(run.status));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, get_status_bg_color(run.status));

    bool is_open = ImGui::CollapsingHeader((run.test_key + ": " + run.test_summary).c_str(),
                                         ImGuiTreeNodeFlags_DefaultOpen);

    ImVec2 header_end_pos = ImGui::GetCursorScreenPos();

    float header_height = header_end_pos.y - header_start_pos.y;

    header_end_pos.x = header_start_pos.x + ImGui::GetContentRegionAvail().x;
    header_end_pos.y = header_start_pos.y + header_height;

    ImGui::PopStyleColor(3);

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
    ImGui::TextColored(get_status_color(run.status), "%s", get_status_string(run.status).c_str());

    bool is_header_hovered = ImGui::IsMouseHoveringRect(header_start_pos, header_end_pos);
    if ((is_header_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))) {
        ImGui::OpenPopup(("test_run_context_menu_" + run.test_key).c_str());
    }

    if (ImGui::BeginPopup(("test_run_context_menu_" + run.test_key).c_str())) {
        ImGui::Text("Set Test Status:");
        ImGui::Separator();

        for (int i = 1; i < static_cast<int>(Status::LENGTH); i++) {
            const Status status = static_cast<Status>(i);
            const std::string& name = get_status_string(status);

            ImVec4 color = get_status_color(status);
            ImGui::PushStyleColor(ImGuiCol_Text, color);

            bool is_selected = (run.status == status);
            if (ImGui::MenuItem(name.c_str(), nullptr, is_selected)) {
                run.status = status;
            }

            ImGui::PopStyleColor();
        }

        ImGui::EndPopup();
    }

    if (is_open) {
        ImGui::Indent(12);

        for (int step_idx = 0; step_idx < run.steps.size(); step_idx++) {
            render_test_step(run, step_idx);

            if (step_idx < run.steps.size() - 1) {
                ImGui::Separator();
            }
        }

        ImGui::Unindent(12);
    }

    ImGui::Dummy(ImVec2(0, 4));
    ImGui::PopID();
}

void TestViewer::render_test_step(TestRun& run, int step_idx) {
    TestStep& step = run.steps[step_idx];

    ImGui::PushID(step_idx);

    ImVec2 header_start_pos = ImGui::GetCursorScreenPos();

    ImGui::PushStyleColor(ImGuiCol_Header, get_status_bg_color(step.status));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, get_status_bg_color(step.status));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, get_status_bg_color(step.status));

    bool is_open = ImGui::CollapsingHeader(("Step " + std::to_string(step_idx + 1)).c_str());

    ImVec2 header_end_pos = ImGui::GetCursorScreenPos();
    float header_height = header_end_pos.y - header_start_pos.y;
    header_end_pos.x = header_start_pos.x + ImGui::GetContentRegionAvail().x;
    header_end_pos.y = header_start_pos.y + header_height;

    ImGui::PopStyleColor(3);

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
    ImGui::TextColored(get_status_color(step.status), "%s", get_status_string(step.status).c_str());

    bool is_header_hovered = ImGui::IsMouseHoveringRect(header_start_pos, header_end_pos);
    if (is_header_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(("test_step_context_menu_" + step.id).c_str());
    }

    if (ImGui::BeginPopup(("test_step_context_menu_" + step.id).c_str())) {
        ImGui::Text("Set Step Status:");
        ImGui::Separator();

        for (int i = 1; i < static_cast<int>(Status::LENGTH); i++) {
            const Status status = static_cast<Status>(i);
            const std::string& name = get_status_string(status);

            ImVec4 color = get_status_color(status);
            ImGui::PushStyleColor(ImGuiCol_Text, color);

            bool is_selected = (step.status == status);
            if (ImGui::MenuItem(name.c_str(), nullptr, is_selected)) {
                step.status = status;
            }

            ImGui::PopStyleColor();
        }

        ImGui::EndPopup();
    }

    if (is_open) {
        ImGui::Indent(8);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.9f, 1.0f, 1.0f));
        ImGui::TextUnformatted("Action:");
        ImGui::PopStyleColor();

        ImGui::BeginChild(("action_text_" + step.id).c_str(),
                        ImVec2(ImGui::GetContentRegionAvail().x - 8, ImGui::GetTextLineHeightWithSpacing() * 3),
                        true);
        ImGui::TextWrapped("%s", step.action.c_str());
        ImGui::EndChild();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.9f, 1.0f, 1.0f));
        ImGui::TextUnformatted("Expected Result:");
        ImGui::PopStyleColor();

        ImGui::BeginChild(("expected_result_text_" + step.id).c_str(),
                        ImVec2(ImGui::GetContentRegionAvail().x - 8, ImGui::GetTextLineHeightWithSpacing() * 3),
                        true);
        ImGui::TextWrapped("%s", step.result.c_str());
        ImGui::EndChild();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.9f, 1.0f, 1.0f));
        ImGui::TextUnformatted("Actual Result:");
        ImGui::PopStyleColor();

        auto& buffer = m_actual_result_buffers[step.id];
        if (buffer.empty()) {
            buffer.resize(2048, 0);
            if (!step.actual_result.empty()) {
                strncpy(buffer.data(), step.actual_result.c_str(), buffer.size() - 1);
            }
        }

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.15f, 0.8f));

        if (ImGui::InputTextMultiline(("##actual_result_" + step.id).c_str(),
                                  buffer.data(),
                                  buffer.size(),
                                  ImVec2(ImGui::GetContentRegionAvail().x - 8, ImGui::GetTextLineHeightWithSpacing() * 3),
                                  ImGuiInputTextFlags_AllowTabInput)) {
            step.actual_result = buffer.data();
        }

        ImGui::PopStyleColor();

        ImGui::Unindent(8);
    }

    ImGui::PopID();
}

void TestViewer::reset_all_tests() {
    for (auto& run : m_test_execution.test_runs) {
        run.status = Status::TODO;
        
        for (auto& step : run.steps) {
            step.status = Status::TODO;
            step.actual_result.clear();
            step.comment.clear();
            step.defects.clear();
        }
    }
    
    m_actual_result_buffers.clear();
}

void TestViewer::load_tests_for_plan(const TestPlanOverview& plan) {
    m_test_execution.key = plan.key + "-EXEC";
    m_test_execution.summary = plan.summary;
    m_test_execution.description = "Test execution for " + plan.summary;

}

void TestViewer::create_mockup_data() {
    TestPlanOverview plan1;
    plan1.key = "TESTPLAN-101";
    plan1.summary = "Browser Compatibility Test Plan";
    plan1.test_count = 3;
    plan1.test_keys = {"TEST-001", "TEST-002", "TEST-003"};
    plan1.test_summaries = {
        "Verify main menu navigation",
        "Test file saving functionality",
        "Verify entity creation and deletion"
    };
    m_test_plans.push_back(plan1);
    
    TestPlanOverview plan2;
    plan2.key = "TESTPLAN-102";
    plan2.summary = "Performance Test Plan";
    plan2.test_count = 2;
    plan2.test_keys = {"TEST-004", "TEST-005"};
    plan2.test_summaries = {
        "Verify application startup time",
        "Test large file loading performance"
    };
    m_test_plans.push_back(plan2);
    
    TestPlanOverview plan3;
    plan3.key = "TESTPLAN-103";
    plan3.summary = "Security Testing";
    plan3.test_count = 2;
    plan3.test_keys = {"TEST-006", "TEST-007"};
    plan3.test_summaries = {
        "Test input validation and sanitization",
        "Verify file permissions handling"
    };
    m_test_plans.push_back(plan3);
    
    m_test_execution.key = "TESTEX-111";
    m_test_execution.summary = "Browser Compatibility Test Plan";
    m_test_execution.description = "Testing the application across different browsers and platforms";
    
    TestRun run1;
    run1.test_key = "TEST-001";
    run1.test_summary = "Verify main menu navigation";
    
    TestStep step1;
    step1.id = "STEP-001";
    step1.action = "Launch the application";
    step1.result = "Application opens successfully with the main menu visible";
    run1.steps.push_back(step1);
    
    TestStep step2;
    step2.id = "STEP-002";
    step2.action = "Click on each menu item in sequence";
    step2.result = "Each menu item correctly navigates to the corresponding screen";
    run1.steps.push_back(step2);
    
    m_test_execution.test_runs.push_back(run1);
    
    TestRun run2;
    run2.test_key = "TEST-002";
    run2.test_summary = "Test file saving functionality";
    
    TestStep step3;
    step3.id = "STEP-003";
    step3.action = "Create a new document and add content";
    step3.result = "Document is created and content is displayed correctly";
    run2.steps.push_back(step3);
    
    TestStep step4;
    step4.id = "STEP-004";
    step4.action = "Save the document using File > Save";
    step4.result = "Document is saved successfully with confirmation message";
    run2.steps.push_back(step4);
    
    TestStep step5;
    step5.id = "STEP-005";
    step5.action = "Close and reopen the application, then open the saved document";
    step5.result = "Document opens with all content intact";
    run2.steps.push_back(step5);
    
    m_test_execution.test_runs.push_back(run2);
    
    TestRun run3;
    run3.test_key = "TEST-003";
    run3.test_summary = "Verify entity creation and deletion";
    
    TestStep step6;
    step6.id = "STEP-006";
    step6.action = "Create a new entity using the Create button";
    step6.result = "Entity is created and appears in the hierarchy";
    run3.steps.push_back(step6);
    
    TestStep step7;
    step7.id = "STEP-007";
    step7.action = "Add components to the entity";
    step7.result = "Components are successfully added and visible in the inspector";
    run3.steps.push_back(step7);
    
    TestStep step8;
    step8.id = "STEP-008";
    step8.action = "Delete the entity";
    step8.result = "Entity is removed from the hierarchy";
    run3.steps.push_back(step8);
    
    m_test_execution.test_runs.push_back(run3);
}

std::string get_status_string(Status status) {
    switch (status) {
        case Status::NONE: return "None";
        case Status::TODO: return "Todo";
        case Status::PASSED: return "Passed";
        case Status::BLOCKED: return "Blocked";
        case Status::SKIPPED: return "Skipped";
        case Status::ACCEPTABLE: return "Acceptable";
        case Status::FAILED: return "Failed";
        default: return "Unknown";
    }
}

ImVec4 get_status_color(Status status) {
    switch (status) {
        case Status::PASSED: return ImVec4(0.0f, 0.8f, 0.0f, 1.0f); 
        case Status::FAILED: return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); 
        case Status::BLOCKED: return ImVec4(1.0f, 0.5f, 0.0f, 1.0f); 
        case Status::SKIPPED: return ImVec4(0.7f, 0.7f, 0.7f, 1.0f); 
        case Status::ACCEPTABLE: return ImVec4(1.0f, 0.8f, 0.0f, 1.0f); 
        default: return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); 
    }
}

ImVec4 get_status_bg_color(Status status) {
    ImVec4 color = get_status_color(status);
    return ImVec4(
        color.x * 0.5f + 0.5f, 
        color.y * 0.5f + 0.5f,
        color.z * 0.5f + 0.5f,
        0.3f 
    );
}
