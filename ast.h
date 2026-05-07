#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>

using namespace std;

class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp, int& temp_count, int& label_count) const = 0;
};

// Expression node types

class ExprNode : public ASTNode {
protected:
    string node_type; // Type information (int, float, void, etc.)
public:
    ExprNode(string type) : node_type(type) {}
    virtual string get_type() const { return node_type; }
};

// Variable node (for ID references)

class VarNode : public ExprNode {
private:
    string name;
    ExprNode* index; // For array access, nullptr for simple variables

public:
    VarNode(string name, string type, ExprNode* idx = nullptr)
        : ExprNode(type), name(name), index(idx) {}
    
    ~VarNode() { if(index) delete index; }
    
    bool has_index() const { return index != nullptr; }
    
    string generate_index_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                              int& temp_count, int& label_count) const {
        // TODO: Implement this method
        // Should generate code to calculate the array index and return the temp variable
        
        // Generate code for the index expression (e.g., 'i' in a[i])
        string idx_temp = index-> generate_code(outcode, symbol_to_temp, temp_count, label_count); 

        // Determine size of the data type (assuming int = 4 bytes, float/double = 8)
        int scale_factor = 4;
        if (node_type == "float" || node_type == "double") {
            scale_factor = 8;
        }

        // Generate temp variable for scaled index: tX = idx_temp * scale_factor
        string scaled_idx_temp = "t" + to_string(temp_count++);
        outcode << scaled_idx_temp << " = " << idx_temp << " * " << scale_factor << endl;

        return scaled_idx_temp;
}
    
        string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
            int& temp_count, int& label_count) const override {
        if (has_index()) {
        // For array access: compute address offset and load value
        string offset_temp = generate_index_code(outcode, symbol_to_temp, temp_count, label_count);
        string result_temp = "t" + to_string(temp_count++);
        outcode << result_temp << " = " << name << "[" << offset_temp << "]" << endl;
        return result_temp;
        } else {
        // Always load simple variables into a temp first
        string result_temp = "t" + to_string(temp_count++);
        outcode << result_temp << " = " << name << endl;
        return result_temp;
        }
        }
    
    string get_name() const { return name; }
};

// Constant node

class ConstNode : public ExprNode {
private:
    string value;

public:
    ConstNode(string val, string type) : ExprNode(type), value(val) {}
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for constant values
        // Create a temporary variable to hold the constant value
        string temp = "t" + to_string(temp_count++);
        // Emit the three-address code: temp = value
        outcode << temp << " = " << value << endl;

        // Return the temp variable that holds the constant
        return temp;
    }
};

// Binary operation node

class BinaryOpNode : public ExprNode {
private:
    string op;
    ExprNode* left;
    ExprNode* right;

public:
    BinaryOpNode(string op, ExprNode* left, ExprNode* right, string result_type)
        : ExprNode(result_type), op(op), left(left), right(right) {}
    
    ~BinaryOpNode() {
        delete left;
        delete right;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for binary operations
        // Generate code for the left and right expressions
        string left_temp = left->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        string right_temp = right->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        // Generate a new temp to hold the result of the binary operation
        string result_temp = "t" + to_string(temp_count++);

        // Emit the three-address code
        outcode << result_temp << " = " << left_temp << " " << op << " " << right_temp << endl;

        return result_temp;
    }
};

// Unary operation node

class UnaryOpNode : public ExprNode {
private:
    string op;
    ExprNode* expr;

public:
    UnaryOpNode(string op, ExprNode* expr, string result_type)
        : ExprNode(result_type), op(op), expr(expr) {}
    
    ~UnaryOpNode() { delete expr; }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for unary operations
        // Generate code for the operand expression
        string expr_temp = expr->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        // Allocate a new temporary variable for the result
        string result_temp = "t" + to_string(temp_count++);

        // Emit the three-address code
        outcode << result_temp << " = " << op << expr_temp << endl;

        return result_temp;
    }
};

// Assignment node

class AssignNode : public ExprNode {
private:
    VarNode* lhs;
    ExprNode* rhs;

public:
    AssignNode(VarNode* lhs, ExprNode* rhs, string result_type)
        : ExprNode(result_type), lhs(lhs), rhs(rhs) {}
    
    ~AssignNode() {
        delete lhs;
        delete rhs;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for assignment operations
        // Generate code for the right-hand side expression
        string rhs_temp = rhs->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        // Handle array assignment
        if (lhs->has_index()) {
            string index_temp = lhs->generate_index_code(outcode, symbol_to_temp, temp_count, label_count);
            outcode << lhs->get_name() << "[" << index_temp << "] = " << rhs_temp << endl;
        } 
        // Handle normal variable assignment
        else {
            string lhs_name = lhs->get_name();
            outcode << lhs_name << " = " << rhs_temp << endl;
            symbol_to_temp[lhs_name] = rhs_temp;
        }

        return ""; // Assignment has no resulting temp for now
    }
};

// Statement node types

class StmtNode : public ASTNode {
public:
    virtual string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                                int& temp_count, int& label_count) const = 0;
};

// Expression statement node

class ExprStmtNode : public StmtNode {
private:
    ExprNode* expr;

public:
    ExprStmtNode(ExprNode* e) : expr(e) {}
    ~ExprStmtNode() { if(expr) delete expr; }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for expression statements
        if (expr) {
            expr->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        }
        return ""; // No result needed for expression statement
    }
};

// Block (compound statement) node

class BlockNode : public StmtNode {
private:
    vector<StmtNode*> statements;

public:
    ~BlockNode() {
        for (auto stmt : statements) {
            delete stmt;
        }
    }
    
    void add_statement(StmtNode* stmt) {
        if (stmt) statements.push_back(stmt);
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for all statements in the block
        for (const auto& stmt : statements) {
            if (stmt) {
                stmt->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            }
        }
        return ""; // No resulting temp needed; it's a statement block
    }
};

// If statement node

class IfNode : public StmtNode {
private:
    ExprNode* condition;
    StmtNode* then_block;
    StmtNode* else_block; // nullptr if no else part

public:
    IfNode(ExprNode* cond, StmtNode* then_stmt, StmtNode* else_stmt = nullptr)
        : condition(cond), then_block(then_stmt), else_block(else_stmt) {}
    
    ~IfNode() {
        delete condition;
        delete then_block;
        if (else_block) delete else_block;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for if-else statements
        // Generate condition code
        string cond_temp = condition->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        // Create unique labels
        string label_then = "L" + to_string(label_count++);
        string label_else = else_block ? "L" + to_string(label_count++) : "";
        string label_end  = "L" + to_string(label_count++);

        // Conditional jump
        if (else_block) {
            outcode << "if " << cond_temp << " goto " << label_then << endl;
            outcode << "goto " << label_else << endl;
        } else {
            outcode << "ifFalse " << cond_temp << " goto " << label_end << endl;
        }

        // Then block
        outcode << label_then << ":" << endl;
        then_block->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        outcode << "goto " << label_end << endl;

        // Else block
        if (else_block) {
            outcode << label_else << ":" << endl;
            else_block->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        }

        // End label
        outcode << label_end << ":" << endl;

        return ""; // No result temp needed for if statement
    }
};

// While statement node

class WhileNode : public StmtNode {
private:
    ExprNode* condition;
    StmtNode* body;

public:
    WhileNode(ExprNode* cond, StmtNode* body_stmt)
        : condition(cond), body(body_stmt) {}
    
    ~WhileNode() {
        delete condition;
        delete body;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
        int& temp_count, int& label_count) const override {
        string label_start = "L" + to_string(label_count++);
        string label_body  = "L" + to_string(label_count++);
        string label_end   = "L" + to_string(label_count++);

        // Label for the start of the condition check
        outcode << label_start << ":" << endl;

        // Generate condition code
        string cond_temp = condition->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        // Conditional jump
        outcode << "if " << cond_temp << " goto " << label_body << endl;
        outcode << "goto " << label_end << endl;

        // Loop body
        outcode << label_body << ":" << endl;
        body->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        // Go back to condition
        outcode << "goto " << label_start << endl;

        // End label
        outcode << label_end << ":" << endl;

        return ""; // No temp result needed for while loops
        }
};

// For statement node

class ForNode : public StmtNode {
private:
    ExprNode* init;
    ExprNode* condition;
    ExprNode* update;
    StmtNode* body;

public:
    ForNode(ExprNode* init_expr, ExprNode* cond_expr, ExprNode* update_expr, StmtNode* body_stmt)
        : init(init_expr), condition(cond_expr), update(update_expr), body(body_stmt) {}
    
    ~ForNode() {
        if (init) delete init;
        if (condition) delete condition;
        if (update) delete update;
        delete body;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
        int& temp_count, int& label_count) const override {
        string label_cond = "L" + to_string(label_count++);
        string label_body = "L" + to_string(label_count++);
        string label_update = "L" + to_string(label_count++);
        string label_end = "L" + to_string(label_count++);

        // Generate init expression code (e.g., i = 0)
        if (init)
        init->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        // Condition check label
        outcode << label_cond << ":" << endl;

        // Generate condition code (e.g., i < 10)
        string cond_temp = condition ? condition->generate_code(outcode, symbol_to_temp, temp_count, label_count) : "1";

        // Branching
        outcode << "if " << cond_temp << " goto " << label_body << endl;
        outcode << "goto " << label_end << endl;

        // Body label
        outcode << label_body << ":" << endl;
        body->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        // Update label (e.g., i = i + 1)
        outcode << label_update << ":" << endl;
        if (update)
        update->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        // Go back to condition check
        outcode << "goto " << label_cond << endl;

        // End of loop
        outcode << label_end << ":" << endl;

        return "";
        }
};

// Return statement node

class ReturnNode : public StmtNode {
private:
    ExprNode* expr;

public:
    ReturnNode(ExprNode* e) : expr(e) {}
    ~ReturnNode() { if (expr) delete expr; }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
        int& temp_count, int& label_count) const override {
        if (expr) {
        string temp = expr->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        outcode << "return " << temp << endl;
        } else {
        outcode << "return" << endl;
        }
        return "";
        }
};

// Declaration node

class DeclNode : public StmtNode {
private:
    string type;
    vector<pair<string, int>> vars; // Variable name and array size (0 for regular vars)

public:
    DeclNode(string t) : type(t) {}
    
    void add_var(string name, int array_size = 0) {
        vars.push_back(make_pair(name, array_size));
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
        int& temp_count, int& label_count) const override {
        for (const auto& var : vars) {
        const string& name = var.first;
        int size = var.second;

        if (size > 0) {
        // Array declaration
        outcode << type << " " << name << "[" << size << "]" << endl;
        } else {
        // Regular variable declaration
        outcode << type << " " << name << endl;
        }

        // Optionally initialize symbol_to_temp
        symbol_to_temp[name] = name;
        }
        return "";
        }
    
    string get_type() const { return type; }
    const vector<pair<string, int>>& get_vars() const { return vars; }
};

// Function declaration node

class FuncDeclNode : public ASTNode {
    private:
        string return_type;
        string name;
        vector<pair<string, string>> params; // Parameter type and name
        BlockNode* body;
    
    public:
        FuncDeclNode(string ret_type, string n) : return_type(ret_type), name(n), body(nullptr) {}
        ~FuncDeclNode() { if (body) delete body; }
    
        // Getter for the function name
        string get_name() const { return name; }
    
        // Getter for the function parameters
        const vector<pair<string, string>>& get_params() const { return params; }
    
        // Getter for the function body
        BlockNode* get_body() const { return body; }
    
        void add_param(string type, string name) {
            params.push_back(make_pair(type, name));
        }
        
        void set_body(BlockNode* b) {
            body = b;
        }
    
string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                     int& temp_count, int& label_count) const override {
    // Skip empty function declarations (no body)
    if (body == nullptr) return "";



    // Declare parameters
    for (const auto& param : params) {
        const string& param_type = param.first;
        const string& param_name = param.second;
        outcode << param_type << " " << param_name << endl;
        symbol_to_temp[param_name] = param_name;
    }

    // Generate body
    body->generate_code(outcode, symbol_to_temp, temp_count, label_count);


    return "";
}
    };

// Helper class for function arguments

class ArgumentsNode : public ASTNode {
private:
    vector<ExprNode*> args;

public:
    ~ArgumentsNode() {
        // Don't delete args here - they'll be transferred to FuncCallNode
    }
    
    void add_argument(ExprNode* arg) {
        if (arg) args.push_back(arg);
    }
    
    ExprNode* get_argument(int index) const {
        if (index >= 0 && index < args.size()) {
            return args[index];
        }
        return nullptr;
    }
    
    size_t size() const {
        return args.size();
    }
    
    const vector<ExprNode*>& get_arguments() const {
        return args;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // This node doesn't generate code directly
        return "";
    }
};

// Function call node

class FuncCallNode : public ExprNode {
    private:
        string func_name;
        vector<ExprNode*> arguments;
    
    public:
        FuncCallNode(string name, string result_type)
            : ExprNode(result_type), func_name(name) {}
        
        ~FuncCallNode() {
            for (auto arg : arguments) {
                delete arg;
            }
        }
        
        void add_argument(ExprNode* arg) {
            if (arg) arguments.push_back(arg);
        }
        
        string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                             int& temp_count, int& label_count) const override {
            vector<string> arg_temps;
    
            // Evaluate all arguments and store their temps
            for (ExprNode* arg : arguments) {
                string temp = arg->generate_code(outcode, symbol_to_temp, temp_count, label_count);
                arg_temps.push_back(temp);
            }
    
            // Pass arguments to function
            for (const string& temp : arg_temps) {
                outcode << "param " << temp << endl;
            }
    
            // Generate a temp variable to hold the return value from the function call
            string result_temp = "t" + to_string(temp_count++);
            outcode << result_temp << " = call " << func_name << ", " << arg_temps.size() << endl;
    
            // Return the temporary variable that holds the result of the function call
            return result_temp;
        }
    };

// Program node (root of AST)
class ProgramNode : public ASTNode {
    private:
        vector<ASTNode*> units; // This holds all units like function declarations and statements
        vector<FuncDeclNode*> functions; // This will hold function declarations
        vector<pair<string, string>> declarations; // This will hold variable declarations
    
    public:
        ~ProgramNode() {
            for (auto unit : units) {
                delete unit;
            }
            // Clean up functions and declarations (if needed)
            for (auto func : functions) {
                delete func;
            }
        }
    
        // Getter for all function nodes
        const vector<FuncDeclNode*>& get_functions() const {
            return functions;
        }
    
        // Getter for all declarations
        const vector<pair<string, string>>& get_declarations() const {
            return declarations;
        }
    
        // Getter for all units (statements) in the program
        const vector<ASTNode*>& get_statements() const {
            return units;
        }
    
        void add_unit(ASTNode* unit) {
            if (!unit) return;
        
            // If it's a function, store it in `functions` only
            if (auto func = dynamic_cast<FuncDeclNode*>(unit)) {
                functions.push_back(func);
                return;
            }
        
            // Otherwise, it's a statement — store in `units`
            units.push_back(unit);
        }
        
    
        string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
            int& temp_count, int& label_count) const override {
        for (FuncDeclNode* func : functions) {
        func->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        }

        for (ASTNode* stmt : units) {
        stmt->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        }

        return "";
        }
    };
    

#endif // AST_H