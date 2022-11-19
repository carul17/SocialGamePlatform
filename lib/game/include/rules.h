#pragma once

#include "list.h"

#include <vector>
#include <functional>
#include "ASTVisitor.h"
#include "ExpressionResolver.h"

class Rule;
typedef std::shared_ptr<Rule> RuleSptr; // shared pointer to a rule object
typedef std::vector<std::shared_ptr<Rule>> RuleVector; // a vector of shared pointers to rule objects

// Rule Interface //

class Rule {
public:
    virtual ~Rule() {}

    bool execute(ElementMap elementsMap, ElementSptr element = nullptr) {
        if (executed) {
            return true;
        }
        return executeImpl(element, elementsMap);
    }
    void reset() {
        executed = false;
        resetImpl();
    }

//for testing
ElementSptr getList();

ExpressionResolver resolver;

private:
    virtual bool executeImpl(ElementSptr element, ElementMap elementsMap) = 0;
    virtual void resetImpl() {}
    
protected:
    bool executed = false;
};

// Control Structures//

class Foreach : public Rule {
private:
    ElementSptr list;
    ElementVector elements;
    ElementVector::iterator element;
    RuleVector rules;
    RuleVector::iterator rule;
    bool initialized = false;

    std::shared_ptr<ASTNode> listExpressionRoot;
    std::string elementName;
public:
    ElementSptr getList();
    Foreach(ElementSptr list, RuleVector rules);
    Foreach(std::shared_ptr<ASTNode> listExpressionRoot, RuleVector _rules, std::string elementName);
    bool executeImpl(ElementSptr element, ElementMap elementsMap) final;
    void resetImpl() final;
};

class ParallelFor : public Rule {
    std::shared_ptr<PlayerMap> player_maps;
    RuleVector rules;
    std::map<Connection, RuleVector::iterator> player_rule_it;
    bool initialized = false;
    
    std::string elementName;
public:
    ParallelFor(std::shared_ptr<PlayerMap> player_maps, RuleVector rules);
    ParallelFor(std::shared_ptr<PlayerMap> player_maps, RuleVector rules, std::string elementName);
    bool executeImpl(ElementSptr element, ElementMap elementsMap) final;
    void resetImpl() final;
};

class When : public Rule {
    // a vector of case-rules pairs
    // containes a rule list for every case
    // a case is a function (lambda) that returns a bool
    std::vector<std::pair<std::function<bool(ElementSptr)>,RuleVector>> case_rules; 
    std::vector<std::pair<std::function<bool(ElementSptr)>,RuleVector>>::iterator case_rule_pair;
    RuleVector::iterator rule;
    bool match = false;

    std::vector<std::pair<std::shared_ptr<ASTNode>, RuleVector>> conditionExpression_rule_pair;
public: 
    When(std::vector<std::pair<std::function<bool(ElementSptr)>,RuleVector>> case_rules);
    When(std::vector<std::pair<std::shared_ptr<ASTNode>, RuleVector>> conditonExpression_rule_pair);
    bool executeImpl(ElementSptr element, ElementMap elementsMap) final;
    void resetImpl() final;
};

// List Operations //

class Extend : public Rule {
    ElementSptr target;
    std::function<ElementSptr(ElementSptr)> extension;

    std::shared_ptr<ASTNode> targetExpressionRoot;
    std::shared_ptr<ASTNode> extensionExpressionRoot;

    ElementSptr extensionList;
public:
    Extend(ElementSptr target, std::function<ElementSptr(ElementSptr)> extension);
    Extend(std::shared_ptr<ASTNode> targetExpressionRoot, std::shared_ptr<ASTNode> extensionExpressionRoot);
    bool executeImpl(ElementSptr element, ElementMap elementsMap) final;
};

class Discard : public Rule {
    ElementSptr list;
    std::function<size_t(ElementSptr)> count;

    std::shared_ptr<ASTNode> listExpressionRoot;
    std::shared_ptr<ASTNode> countExpressionRoot;
    int resolvedCount;
public:
    Discard(ElementSptr list, std::function<size_t(ElementSptr)> count);
    Discard(std::shared_ptr<ASTNode> listExpressionRoot,  std::shared_ptr<ASTNode> countExpressionRoot);
    bool executeImpl(ElementSptr element, ElementMap elementsMap) final;
};

// Arithmetic //

class Add : public Rule {
    std::string to;
    ElementSptr value;
public: 
    Add(std::string to, ElementSptr value);
    bool executeImpl(ElementSptr element, ElementMap elementsMap) final;
};

// Input/ Output //

class InputChoice : public Rule {
    std::string prompt;
    ElementVector choices;
    unsigned timeout_s; // in seconds
    std::string result;
    std::shared_ptr<std::deque<Message>> player_msgs;
    std::shared_ptr<std::map<Connection, std::string>> player_input;
    std::map<Connection, bool> alreadySentInput;

    std::shared_ptr<ASTNode> choicesExpressionRoot;
public:
    InputChoice(std::string prompt, ElementVector choices, 
                unsigned timeout_s, std::string result,
                std::shared_ptr<std::deque<Message>> player_msgs,
                std::shared_ptr<std::map<Connection, std::string>> player_input);
    InputChoice(std::string prompt, std::shared_ptr<ASTNode> expressionRoot, 
                unsigned timeout_s, std::string result,
                std::shared_ptr<std::deque<Message>> player_msgs,
                std::shared_ptr<std::map<Connection, std::string>> player_input);
    bool executeImpl(ElementSptr element, ElementMap elementsMap) final;
};

class GlobalMsg : public Rule {
    std::string msg;
    std::shared_ptr<std::deque<std::string>> global_msgs;
public:
    GlobalMsg(std::string msg,
              std::shared_ptr<std::deque<std::string>> global_msgs);
    bool executeImpl(ElementSptr element, ElementMap elementsMap) final;
};

class Scores : public Rule {
    std::shared_ptr<PlayerMap> player_maps;
    std::string attribute_key;
    bool ascending;
    std::shared_ptr<std::deque<std::string>> global_msgs;
public:
    Scores(std::shared_ptr<PlayerMap> player_maps, std::string attribute_key, 
           bool ascending, std::shared_ptr<std::deque<std::string>> global_msgs);
    bool executeImpl(ElementSptr element, ElementMap elementsMap) final;
};