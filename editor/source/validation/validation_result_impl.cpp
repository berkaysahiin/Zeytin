module;

#include <string>
#include <vector>
#include <sstream>

module zeytin.validation;

ValidationResult::ValidationResult(bool valid, std::vector<std::string> errors)
    : m_is_valid(valid)
    , m_errors(std::move(errors)) {
}

ValidationResult ValidationResult::success() {
    return ValidationResult(true, {});
}

ValidationResult ValidationResult::failure(std::string error) {
    std::vector<std::string> errors;
    errors.push_back(std::move(error));
    return ValidationResult(false, std::move(errors));
}

ValidationResult ValidationResult::failure(std::vector<std::string> errors) {
    return ValidationResult(false, std::move(errors));
}

bool ValidationResult::is_valid() const {
    return m_is_valid;
}

bool ValidationResult::is_invalid() const {
    return !m_is_valid;
}

const std::vector<std::string>& ValidationResult::get_errors() const {
    return m_errors;
}

std::string ValidationResult::get_first_error() const {
    if (m_errors.empty()) {
        return "";
    }
    return m_errors[0];
}

std::string ValidationResult::get_all_errors() const {
    if (m_errors.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < m_errors.size(); ++i) {
        oss << m_errors[i];
        if (i < m_errors.size() - 1) {
            oss << "\n";
        }
    }
    return oss.str();
}

void ValidationResult::add_error(std::string error) {
    m_errors.push_back(std::move(error));
    m_is_valid = false;
}

void ValidationResult::merge(const ValidationResult& other) {
    if (other.is_invalid()) {
        m_is_valid = false;
        m_errors.insert(m_errors.end(), other.m_errors.begin(), other.m_errors.end());
    }
}

ValidationResult::operator bool() const {
    return m_is_valid;
}
