// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <new>

namespace {

    enum class token_type 
    {
        nil,
        lbrace,
        rbrace,
        key,
        expression
    };

    struct lbrace_arg_t
    {
        explicit lbrace_arg_t() = default;
    };
    constexpr lbrace_arg_t lbrace_arg{};

    struct rbrace_arg_t
    {
        explicit rbrace_arg_t() = default;
    };
    constexpr rbrace_arg_t rbrace_arg{};

    struct expression
    {
    };

    class token
    {
    public:
        token_type type_;

        union
        {
            std::unique_ptr<expression> expr_;
            std::string key_;
        };
    public:

        token(lbrace_arg_t)
            : type_(token_type::lbrace)
        {
        }

        token(rbrace_arg_t)
            : type_(token_type::rbrace)
        {
        }

        token(const std::string& key)
            : type_(token_type::key)
        {
            new (&key_) std::string(key);
        }

        token(const std::unique_ptr<expression>& expr) = delete;

        token(std::unique_ptr<expression>&& expr)
            : type_(token_type::expression)
        {
            new (&expr_) std::unique_ptr<expression>(std::move(expr));
        }

        token(token&& other)
        {
            construct(std::forward<token>(other));
        }

        token& operator=(const token& other) = delete;

        token& operator=(token&& other)
        {
            if (&other != this)
            {
                if (type_ == other.type_)
                {
                    switch (type_)
                    {
                        case token_type::expression:
                            expr_ = std::move(other.expr_);
                            break;
                        case token_type::key:
                            key_ = std::move(other.key_);
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    destroy();
                    construct(std::forward<token>(other));
                }
            }
            return *this;
        }

        ~token() noexcept
        {
            destroy();
        }

        token_type type() const
        {
            return type_;
        }

        void construct(token&& other)
        {
            type_ = other.type_;
            switch (type_)
            {
                case token_type::expression:
                    new (&expr_) std::unique_ptr<expression>(std::move(other.expr_));
                    break;
                case token_type::key:
                    new (&key_) std::string(std::move(other.key_));
                    break;
                default:
                    break;
            }
        }

        void destroy() noexcept 
        {
            switch(type_)
            {
                case token_type::expression:
                    expr_.~unique_ptr();
                    break;
                case token_type::key:
                    key_.~basic_string();
                    break;
                default:
                    break;
            }
        }
    };

    std::vector<token> output_stack;

    void push_token(token&& tok)
    {
        switch (tok.type())
        {
            case token_type::lbrace:
            case token_type::key:
            case token_type::expression:
                output_stack.push_back(std::move(tok));
                break;
            case token_type::rbrace:
            {
                auto it = output_stack.rbegin();
                while (it != output_stack.rend() && it->type() != token_type::lbrace)
                {
                    assert(it->type() == token_type::expression);
                    auto expr = std::move(output_stack.back().expr_);
                    ++it;
                    assert(it->type() == token_type::key);
                    auto key = std::move(output_stack.back().key_);
                    ++it;
                }
                assert(it != output_stack.rend());
                output_stack.erase(it.base(),output_stack.end());
                assert(it->type() == token_type::lbrace);
                output_stack.pop_back();
                break;
            }
        }
    }
}

TEST_CASE("jmespath::token tests")
{
    SECTION("test2")
    {
        push_token(token(lbrace_arg));
        push_token(token("foo"));
        push_token(token(std::make_unique<expression>()));
        push_token(token(rbrace_arg));
    }
}

