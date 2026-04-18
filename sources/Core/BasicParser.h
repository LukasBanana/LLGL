/*
 * BasicParser.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_BASIC_PARSER_H
#define LLGL_BASIC_PARSER_H


#include <LLGL/Container/ArrayView.h>


namespace LLGL
{


// Template for basic token parsers.
template <typename TToken>
class BasicParser
{

    public:

        BasicParser() = default;

        BasicParser(ArrayView<TToken> tokens) :
            tokens_ { tokens }
        {
        }

    public:

        // Resets the internal token iterator.
        void Reset()
        {
            iter_ = 0;
        }

        // Returns the current token with an optional offset.
        const TToken& Token(int offset = 0) const
        {
            if (offset < 0)
            {
                unsigned uOffset = static_cast<unsigned>(-offset);
                if (uOffset < iter_ && iter_ - uOffset < tokens_.size())
                    return tokens_[iter_ - uOffset];
            }
            else if (offset > 0)
            {
                unsigned uOffset = static_cast<unsigned>(offset);
                if (iter_ + uOffset < tokens_.size())
                    return tokens_[iter_ + uOffset];
            }
            else if (iter_ < tokens_.size())
            {
                /* Return current token */
                return tokens_[iter_];
            }
            return GetNullToken();
        }

        // Accepts and returns the current token, then moves to the next token.
        const TToken& Accept()
        {
            if (iter_ < tokens_.size())
            {
                const TToken& tok = Token();
                ++iter_;
                return tok;
            }
            return GetNullToken();
        }

        // Returns true if there are further tokens to parse.
        bool Feed() const
        {
            return (iter_ < tokens_.size());
        }

        // Returns an array view of all tokens.
        const ArrayView<TToken>& GetTokens() const
        {
            return tokens_;
        }

        // Returns the current token iterator.
        std::size_t GetIterator() const
        {
            return iter_;
        }

    private:

        static const TToken& GetNullToken()
        {
            static const TToken kNullToken = {};
            return kNullToken;
        }

    private:

        ArrayView<TToken>   tokens_;
        std::size_t         iter_   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
