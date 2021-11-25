#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H


#include <vector>
#include "Python3BaseVisitor.h"
#include "Scope.h"
#include "Exception.h"
#include "utils.h"
#include "BaseType.h"

#include <iostream>
using std::cin;
using std::cout;
using std::endl;

class EvalVisitor: public Python3BaseVisitor {

    Scope scope;

    virtual antlrcpp::Any visitFile_input(Python3Parser::File_inputContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFuncdef(Python3Parser::FuncdefContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitParameters(Python3Parser::ParametersContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitTfpdef(Python3Parser::TfpdefContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitStmt(Python3Parser::StmtContext *ctx) override {
        if (ctx->simple_stmt())
            return visitSimple_stmt(ctx->simple_stmt());
        else return visitCompound_stmt(ctx->compound_stmt());
    }

    virtual antlrcpp::Any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override {
        return visitSmall_stmt(ctx->small_stmt());
    }

    virtual antlrcpp::Any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override {
        if (ctx->flow_stmt()) return visitFlow_stmt(ctx->flow_stmt());
        else return visitExpr_stmt(ctx->expr_stmt());
    }

    virtual antlrcpp::Any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override {
        // must return 0
        if (ctx->augassign()) {
            // TODO;
            return 0;
        }

        auto testlistArray = ctx->testlist();
        int arraySize = testlistArray.size(); // ! , TODO

        BaseType varData = visitTestlist(testlistArray[arraySize - 1]);

        for (int i = 0; i < arraySize - 1; ++i) {
            std::string varName = testlistArray[i]->getText();
            scope.varRegister(varName, varData);
        }

        return 0;
    }

    virtual antlrcpp::Any visitAugassign(Python3Parser::AugassignContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override {
        if (ctx->break_stmt()) return -1;
        if (ctx->continue_stmt()) return -2;
        if (ctx->return_stmt()) return -3;
    }

    virtual antlrcpp::Any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override {
        auto test = ctx->test();
        auto suite = ctx->suite();
        auto testSize = test.size();
        for (int i = 0; i < testSize; ++i)
            if ((bool) visitTest(test[i]).as<BaseType>()) {
                return visitSuite(suite[i]);
            }
        if (testSize != suite.size()) 
            return visitSuite(suite[testSize]);
        return 0;
    }

    virtual antlrcpp::Any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override {
        while ((bool)visitTest(ctx->test()).as<BaseType>())
            if (visitSuite(ctx->suite()).as<int>())
                break;
        return 0;
    }

    virtual antlrcpp::Any visitSuite(Python3Parser::SuiteContext *ctx) override {
        if (ctx->simple_stmt())
            return visitSimple_stmt(ctx->simple_stmt());
        auto stmt = ctx->stmt();
        for (auto x : stmt) {
            int tmp = visitStmt(x).as<int>();
            if (tmp == -1) return 1;
            if (tmp == -2) return 0;
        }
        return 0;
    }

    virtual antlrcpp::Any visitTest(Python3Parser::TestContext *ctx) override {
        return visitOr_test(ctx->or_test());
    }

    virtual antlrcpp::Any visitOr_test(Python3Parser::Or_testContext *ctx) override {
        auto tmp = ctx->and_test();
        if (tmp.size() == 1)
            return visitAnd_test(tmp[0]);
        for (auto x : tmp)
            if ((bool) visitAnd_test(x))
                return BaseType(true);
        return BaseType(false);
    }

    virtual antlrcpp::Any visitAnd_test(Python3Parser::And_testContext *ctx) override {
        auto tmp = ctx->not_test();
        if (tmp.size() == 1)
            return visitNot_test(tmp[0]);
        for (auto x : tmp)
            if (! (bool) visitNot_test(x)) 
                return BaseType(false);
        return BaseType(true);
    }

    virtual antlrcpp::Any visitNot_test(Python3Parser::Not_testContext *ctx) override {
        if (ctx->NOT()) return BaseType(!(bool)visitNot_test(ctx->not_test()).as<BaseType>());
        else return visitComparison(ctx->comparison());
    }

    bool mycmp(const BaseType &lhs, const BaseType &rhs, const string &opt) {
        if (opt == "<") return lhs < rhs;
        if (opt == ">") return lhs > rhs;
        if (opt == "==") return lhs == rhs;
        if (opt == ">=") return lhs >= rhs;
        if (opt == "<=") return lhs <= rhs;
        if (opt == "!=") return lhs != rhs;
        // '<'|'>'|'=='|'>='|'<=' | '!='
    }

    virtual antlrcpp::Any visitComparison(Python3Parser::ComparisonContext *ctx) override {
        auto vec = ctx->arith_expr();
        auto last = visitArith_expr(vec[0]);
        auto szv = vec.size();
        if (szv == 1) return last;
        auto opt = ctx->comp_op();
        for (int i = 1; i < szv; ++i) {
            auto now = visitArith_expr(vec[i]);
            if (!mycmp(last, now, visitComp_op(opt[i - 1]).as<string>()))
                return BaseType(false);
            last = now;
        }
        return BaseType(true);
    }

    virtual antlrcpp::Any visitComp_op(Python3Parser::Comp_opContext *ctx) override {
        return ctx->getText();
    }

    virtual antlrcpp::Any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override {
        auto t = ctx->term();
        BaseType res = visitTerm(t[0]);
        auto szt = t.size();
        if (szt == 1) return res;
        auto o = ctx->addorsub_op();
        for (int i = 1; i < szt; ++i) {
            if ((visitAddorsub_op(o[i - 1])).as<bool>())
                res = res + visitTerm(t[i]);
            else res = res - visitTerm(t[i]);
        }
        return res;
    }

    virtual antlrcpp::Any visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx) override {
        return bool(ctx->getText() == "+");
    }

    virtual antlrcpp::Any visitTerm(Python3Parser::TermContext *ctx) override {
        auto f = ctx->factor();
        BaseType res = visitFactor(f[0]).as<BaseType>();
        auto szf = f.size();
        if (szf == 1) return res; 
        auto o = ctx->muldivmod_op();
        for (int i = 1; i < szf; ++i) {
            string opt = visitMuldivmod_op(o[i - 1]).as<string>();
            if (opt == "*") res = mul(res, visitFactor(f[i]).as<BaseType>());
            if (opt == "/") res = ddiv(res, visitFactor(f[i]).as<BaseType>());
            if (opt == "//") res = idiv(res, visitFactor(f[i]).as<BaseType>());
            if (opt == "%") res = mod(res, visitFactor(f[i]).as<BaseType>());
        }
        return res;
    }

    virtual antlrcpp::Any visitMuldivmod_op(Python3Parser::Muldivmod_opContext *ctx) override {
        return ctx->getText();
    }

    virtual antlrcpp::Any visitFactor(Python3Parser::FactorContext *ctx) override {
        auto atomExpr = ctx->atom_expr();
        if (atomExpr) return visitAtom_expr(atomExpr);
        if (ctx->getText() == "+") return visitFactor(ctx->factor());
        else return BaseType(-visitFactor(ctx->factor()).as<BaseType>());
    }

    virtual antlrcpp::Any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override {
        auto trailer = ctx->trailer();
        if (!trailer) return visitAtom(ctx->atom());
        auto functionName = ctx->atom()->getText();
        auto argsArray = visitTrailer(ctx->trailer()).as<std::vector<BaseType>>();
        if (functionName == "print") {
            for (auto i : argsArray) 
                i.print(' ');
            puts("");
            return BaseType();
        }
        if (functionName == "int") {
            ;
        }
        if (functionName == "float") {
            return BaseType((double) argsArray[0]);
        }
        if (functionName == "str") {
            ;
        }
        if (functionName == "bool") {
            ;
        }
        // TODO
    }

    virtual antlrcpp::Any visitTrailer(Python3Parser::TrailerContext *ctx) override {
        if (ctx->arglist()) return visit(ctx->arglist());
        std::vector<BaseType> res;
        res.clear();
        return res;
    }

    std::pair<bool, double> stringToDouble(const string &number) {
        int idx = -1, sz = number.size();
        for (int i = 0; i < sz; ++i)
            if (number[i] == '.') idx = i;
        if (idx == -1) return std::make_pair(false, 0);
        double res = 0;
        for (int i = idx - 1; i >= 0; --i)
            res = res * 10 + number[i] - '0';

        double tmp = 0.1;
        for (int i = idx + 1; i < sz; ++i) {
            res += tmp * (number[i] - '0');
            tmp /= 10.0;
        }

        return std::make_pair(true, res);
    }

    virtual antlrcpp::Any visitAtom(Python3Parser::AtomContext *ctx) override {
        if (ctx->NUMBER()) {
            std::string number = ctx->NUMBER()->getText();
            std::pair<bool, double> tmp = stringToDouble(number);
            if (tmp.first) return BaseType(tmp.second);
            return BaseType(int2048(number));
        } else if (ctx->NAME()) {
            auto result = scope.varQuery(ctx->NAME()->getText());
            if (result.first) return result.second;
        } else if (ctx->test()) return visitTest(ctx->test());
        else if (ctx->TRUE()) return BaseType(true);
        else if (ctx->FALSE()) return BaseType(false);
        else if (ctx->NONE()) return BaseType();
        else {
            auto s = ctx->STRING();
            string res;
            res.clear();
            for (auto t : s) {
                string tmp = t->getText();
                for (int i = 1, sz = tmp.size(); i < sz - 1; ++i)
                    res += tmp[i];
            }
                
            return BaseType(res);
        }
    }

    virtual antlrcpp::Any visitTestlist(Python3Parser::TestlistContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitArglist(Python3Parser::ArglistContext *ctx) override {
        std::vector<BaseType> res;
        res.clear();
        auto arg = ctx->argument();
        for (auto x : arg)
            res.push_back(visitArgument(x).as<BaseType>());
        return res;
    }

    virtual antlrcpp::Any visitArgument(Python3Parser::ArgumentContext *ctx) override {
        auto test = ctx->test();
        if (test.size() == 1)
            return visitTest(test[0]);
        BaseType varData = visitTest(test[1]);
        string varName = test[0]->getText();
        scope.varRegister(varName, varData);
        return varData;
    }

};


#endif //PYTHON_INTERPRETER_EVALVISITOR_H
