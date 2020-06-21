// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <iostream>
#include <vector>
#include <utility>
#include <new>

namespace {

    enum class token_type 
    {
        lbrace,
        rbrace,
        key,
        expression
    };

    struct lparen_arg_t
    {
        explicit lparen_arg_t() = default;
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

    class identifier_selector final
    {
    private:
        //std::string identifier_;
    public:
        identifier_selector(/*const std::string& name*/)
            //: identifier_(name)
        {
        }
    };

    // token

    class token
    {
    public:
        token_type type_;

        union
        {
            std::unique_ptr<identifier_selector> expression_;
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

        token(std::unique_ptr<identifier_selector> expression)
            : type_(token_type::expression)
        {
            new (&expression_) std::unique_ptr<identifier_selector>(std::move(expression));
        }

        token(token&& other)
            : type_(token_type::lbrace)
        {
            swap(other);
        }

        bool is_lbrace() const
        {
            return type_ == token_type::lbrace; 
        }

        bool is_key() const
        {
            return type_ == token_type::key; 
        }

        bool is_expression() const
        {
            return type_ == token_type::expression; 
        }

        void swap(token& other) noexcept
        {
            if (type_ == other.type_)
            {
                switch (type_)
                {
                    case token_type::lbrace:
                    case token_type::rbrace:
                        break;
                    case token_type::expression:
                        expression_.swap(other.expression_);
                        break;
                    case token_type::key:
                        key_.swap(other.key_);
                        break;
                }
            }
            else
            {
                switch (type_)
                {
                    case token_type::lbrace:
                    case token_type::rbrace:
                        break;
                    case token_type::expression:
                        new (&other.expression_) std::unique_ptr<identifier_selector>(std::move(expression_));
                        break;
                    case token_type::key:
                        new (&other.key_) std::string(std::move(key_));
                        break;
                }
                switch (other.type_)
                {
                    case token_type::lbrace:
                    case token_type::rbrace:
                        break;
                    case token_type::expression:
                        new (&expression_) std::unique_ptr<identifier_selector>(std::move(other.expression_));
                        break;
                    case token_type::key:
                        new (&key_) std::string(std::move(other.key_));
                        break;
                }
                std::swap(type_,other.type_);
            }

        }

        token& operator=(token&& other)
        {
            if (&other != this)
            {
                swap(other);
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

        void destroy() noexcept 
        {
            switch(type_)
            {
                case token_type::expression:
                    if (expression_.get() != nullptr)
                    {
                        expression_.get_deleter()(expression_.get());
                        expression_.reset(nullptr);
                    }
                    break;
                case token_type::key:
                    key_.~basic_string();
                    break;
                default:
                    break;
            }
        }
    };

    std::vector<token> output_stack_;

    void push_token(token&& tok)
    {
        switch (tok.type())
        {
            case token_type::rbrace:
            {
                auto it = output_stack_.rbegin();
                while (it != output_stack_.rend() && !it->is_lbrace())
                {
                    assert(it->is_expression());
                    auto val = std::move(output_stack_.back().expression_);
                    ++it;
                    assert(it->is_key());
                    auto key = std::move(output_stack_.back().key_);
                    ++it;
                }
                assert(it != output_stack_.rend());
                output_stack_.erase(it.base(),output_stack_.end());
                output_stack_.pop_back();

                break;
            }
            case token_type::lbrace:
                output_stack_.emplace_back(std::move(tok));
                break;
            case token_type::key:
                output_stack_.emplace_back(std::move(tok));
                break;
            case token_type::expression:
                output_stack_.emplace_back(std::move(tok));
                break;
        }
    }
}

TEST_CASE("jmespath::token tests")
{
    SECTION("test2")
    {
        std::string buffer("foo");

        push_token(token(lbrace_arg));
        push_token(token(buffer));
        push_token(token(std::make_unique<identifier_selector>()));
        push_token(token(rbrace_arg));
    }
}

