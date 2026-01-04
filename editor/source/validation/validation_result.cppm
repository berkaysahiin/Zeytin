module;

#include <string>
#include <vector>

export module zeytin.validation;

export class ValidationResult {
public:
    static ValidationResult success();
    static ValidationResult failure(std::string error);
    static ValidationResult failure(std::vector<std::string> errors);
    
    bool is_valid() const;
    bool is_invalid() const;
    
    const std::vector<std::string>& get_errors() const;
    std::string get_first_error() const;
    std::string get_all_errors() const; // Concatenated with newlines
    
    void add_error(std::string error);
    void merge(const ValidationResult& other);
    
    explicit operator bool() const;
private:
    ValidationResult(bool valid, std::vector<std::string> errors);
    
    bool m_is_valid;
    std::vector<std::string> m_errors;
};
