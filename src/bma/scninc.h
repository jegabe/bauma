#ifndef BMA_SCNINC_H_INCLUDED
#define BMA_SCNINC_H_INCLUDED

/*
==============================================================================
MIT License

Copyright (c) 2026 Jens Ganter-Benzing

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================

Random utils
*/
#include <time.h>
#include <bma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

/*!
\brief Type for representing the type of a scanned #include statement.
The version with angle brackets (BMA_SCNND_INCLD_STMT_TYPE_SYS_FIRST)
means: Scan system include paths first, then
the passed include directories and NEVER relative to the current file;
The version with double quote (BMA_SCNND_INCLD_STMT_TYPE_LOCAL_FIRST)
means: First try relative to the current file,
then scan the passed include directories first and lastly look at the system include paths.
This is the same as the usual C/C++ compiler behavior.
*/
typedef unsigned int bma_IncldStmtType;

/*! \brief Version for angle bracket includes */
#define BMA_SCNND_INCLD_STMT_TYPE_SYS_FIRST    ((bma_IncldStmtType)0)

/*! \brief Version for double quote includes */
#define BMA_SCNND_INCLD_STMT_TYPE_LOCAL_FIRST  ((bma_IncldStmtType)1)

/*! \brief Structure for representing a scanned #include statement from a C or C++ source file */
typedef struct bma_ScnndIncldStmt {
	bma_IncldStmtType type; /*!< \brief Type of include statement, see bma_IncldStmtType */
	char *pPath; /*!< \brief Path of the include statement, e.g. "myheader.h" or "stdio.h".
	                         Not resolved but just a copy of the found path */
	bma_bool_t isCndtnl; /*!< \brief True when the include appears inside a conditional preprocessor branch. */
} bma_ScnndIncldStmt;

/*! \brief Destructor for bma_ScnndIncldStmt struture. de-allocates the pPath member with pAlloc */
BMA_DEF void bma_ScnndIncldStmt_dtor(bma_ScnndIncldStmt *pSelf, bma_IMemAlloc *pAlloc);

/*! \brief Vector for storing scanned #include statements, which is the result of bma_scnFrInclds */
BMA_DEF_VEC(bma_ScnndIncldStmtVec, bma_ScnndIncldStmt, (bma_dtor_t)&bma_ScnndIncldStmt_dtor)

/*!
\brief Looks a the content of a C or C++ source or header file and detects include preprocessor statements.
The ones found are stored in the output vector. This vector must be already constructed and will not be emptied
before new items are added. Returns BMA_FALSE for malformed literals/comments. Macro-form include operands are
skipped with a warning. When pErrOrWarn is non-NULL, diagnostic descriptions and input offsets are appended to it.
Warnings are also appended when scanning succeeds and returns BMA_TRUE.
Includes inside conditional directives are marked conditional, except inside a conventional #ifndef or
#if !defined(NAME) file guard whose opening directive is the first conditional directive in the file.
*/
BMA_DEF bma_bool_t bma_scnFrInclds_ext(
	const char* pFileContent, size_t fileContentLen, bma_ScnndIncldStmtVec *pOut, bma_StrBldr *pErrOrWarn
);

/*! \brief Convenience macro for calling bma_scnFrInclds_ext with a null-terminated string */
#define bma_scnFrInclds(pFileContent, pOut) \
	bma_scnFrInclds_ext((pFileContent), strlen(pFileContent), (pOut), NULL)

#ifdef __cplusplus
	} /* extern "C" */
#endif

#if (defined(BMA_SCNINC_IMPL) || defined(BMA_IMPL))

#ifdef __cplusplus
	extern "C" {
#endif

BMA_DEF void bma_ScnndIncldStmt_dtor(bma_ScnndIncldStmt *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_assert(pAlloc != NULL);
	bma_free_ext(pAlloc, (void*)pSelf->pPath);
}

typedef unsigned int bma_scnFrInclds_ParseState;
#define BMA_SCN_FR_INCLDS_STATE_NORMAL        ((bma_scnFrInclds_ParseState)0)
#define BMA_SCN_FR_INCLDS_STATE_BLOCK_COMMENT ((bma_scnFrInclds_ParseState)1)
#define BMA_SCN_FR_INCLDS_STATE_LINE_COMMENT  ((bma_scnFrInclds_ParseState)2)
#define BMA_SCN_FR_INCLDS_STATE_STRING        ((bma_scnFrInclds_ParseState)3)
#define BMA_SCN_FR_INCLDS_STATE_CHAR          ((bma_scnFrInclds_ParseState)4)
#define BMA_SCN_FR_INCLDS_STATE_RAW_STRING    ((bma_scnFrInclds_ParseState)5)

BMA_DEF size_t bma_scnFrInclds_newlineLen(const char *pText, size_t textLen, size_t pos) {
	if (pos >= textLen) return 0u;
	if (pText[pos] == '\r') return (pos + 1u < textLen && pText[pos + 1u] == '\n') ? 2u : 1u;
	return pText[pos] == '\n' ? 1u : 0u;
}

BMA_DEF size_t bma_scnFrInclds_spliceLen(const char *pText, size_t textLen, size_t pos) {
	size_t newlineLen;
	if (pos >= textLen || pText[pos] != '\\') return 0u;
	newlineLen = bma_scnFrInclds_newlineLen(pText, textLen, pos + 1u);
	return newlineLen == 0u ? 0u : newlineLen + 1u;
}

BMA_DEF bma_bool_t bma_scnFrInclds_isIdentChar(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
	       (c >= '0' && c <= '9') || c == '_';
}

BMA_DEF bma_bool_t bma_scnFrInclds_skipDirectiveTrivia(
	const char *pText, size_t textLen, size_t *pPos
) {
	size_t pos = *pPos;
	for (;;) {
		size_t spliceLen = bma_scnFrInclds_spliceLen(pText, textLen, pos);
		if (spliceLen != 0u) {
			pos += spliceLen;
			continue;
		}
		if (pos >= textLen) return BMA_FALSE;
		if (pText[pos] == ' ' || pText[pos] == '\t' || pText[pos] == '\v' || pText[pos] == '\f') {
			++pos;
			continue;
		}
		if (bma_scnFrInclds_newlineLen(pText, textLen, pos) != 0u) return BMA_FALSE;
		if (pos + 1u < textLen && pText[pos] == '/' && pText[pos + 1u] == '/') return BMA_FALSE;
		if (pos + 1u < textLen && pText[pos] == '/' && pText[pos + 1u] == '*') {
			pos += 2u;
			while (pos + 1u < textLen && !(pText[pos] == '*' && pText[pos + 1u] == '/')) {
				if (bma_scnFrInclds_newlineLen(pText, textLen, pos) != 0u) return BMA_FALSE;
				++pos;
			}
			if (pos + 1u >= textLen) return BMA_FALSE;
			pos += 2u;
			continue;
		}
		*pPos = pos;
		return BMA_TRUE;
	}
}

BMA_DEF bma_bool_t bma_scnFrInclds_matchDirectiveWord(
	const char *pText, size_t textLen, size_t pos, const char *pWord, size_t *pAfter
) {
	size_t wordPos = 0u;
	while (pWord[wordPos] != '\0') {
		size_t spliceLen;
		while ((spliceLen = bma_scnFrInclds_spliceLen(pText, textLen, pos)) != 0u) pos += spliceLen;
		if (pos >= textLen || pText[pos] != pWord[wordPos]) return BMA_FALSE;
		++pos;
		++wordPos;
	}
	while (bma_scnFrInclds_spliceLen(pText, textLen, pos) != 0u)
		pos += bma_scnFrInclds_spliceLen(pText, textLen, pos);
	if (pos < textLen && bma_scnFrInclds_isIdentChar(pText[pos])) return BMA_FALSE;
	*pAfter = pos;
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_scnFrInclds_matchDirectivePrefix(
	const char *pText, size_t textLen, size_t pos, const char *pPrefix
) {
	size_t prefixPos = 0u;
	while (pPrefix[prefixPos] != '\0') {
		size_t spliceLen;
		while ((spliceLen = bma_scnFrInclds_spliceLen(pText, textLen, pos)) != 0u) pos += spliceLen;
		if (pos >= textLen || pText[pos] != pPrefix[prefixPos]) return BMA_FALSE;
		++pos;
		++prefixPos;
	}
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_scnFrInclds_readIdentifier(
	const char *pText, size_t textLen, size_t *pPos, size_t *pStart, size_t *pLen
) {
	size_t pos = *pPos;
	if (!bma_scnFrInclds_skipDirectiveTrivia(pText, textLen, &pos) || pos >= textLen ||
	    !((pText[pos] >= 'a' && pText[pos] <= 'z') || (pText[pos] >= 'A' && pText[pos] <= 'Z') || pText[pos] == '_'))
		return BMA_FALSE;
	*pStart = pos;
	while (pos < textLen && bma_scnFrInclds_isIdentChar(pText[pos])) ++pos;
	*pLen = pos - *pStart;
	*pPos = pos;
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_scnFrInclds_readNegatedDefinedMacro(
	const char *pText, size_t textLen, size_t pos, size_t *pNameStart, size_t *pNameLen
) {
	size_t namePos;
	size_t nameLen;
	size_t afterWord;
	bma_bool_t hasOuterParens;
	bma_bool_t hasDefinedParens;
	if (!bma_scnFrInclds_skipDirectiveTrivia(pText, textLen, &pos)) return BMA_FALSE;
	hasOuterParens = pos < textLen && pText[pos] == '(';
	if (hasOuterParens) ++pos;
	if (!bma_scnFrInclds_skipDirectiveTrivia(pText, textLen, &pos) || pText[pos] != '!') return BMA_FALSE;
	++pos;
	if (!bma_scnFrInclds_skipDirectiveTrivia(pText, textLen, &pos) ||
	    !bma_scnFrInclds_matchDirectiveWord(pText, textLen, pos, "defined", &afterWord)) return BMA_FALSE;
	pos = afterWord;
	if (!bma_scnFrInclds_skipDirectiveTrivia(pText, textLen, &pos)) return BMA_FALSE;
	hasDefinedParens = pos < textLen && pText[pos] == '(';
	if (hasDefinedParens) ++pos;
	if (!bma_scnFrInclds_readIdentifier(pText, textLen, &pos, &namePos, &nameLen)) return BMA_FALSE;
	if (hasDefinedParens) {
		if (!bma_scnFrInclds_skipDirectiveTrivia(pText, textLen, &pos) || pos >= textLen || pText[pos] != ')')
			return BMA_FALSE;
		++pos;
	}
	if (hasOuterParens) {
		if (!bma_scnFrInclds_skipDirectiveTrivia(pText, textLen, &pos) || pos >= textLen || pText[pos] != ')')
			return BMA_FALSE;
		++pos;
	}
	if (bma_scnFrInclds_skipDirectiveTrivia(pText, textLen, &pos) && pos < textLen) return BMA_FALSE;
	*pNameStart = namePos;
	*pNameLen = nameLen;
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_scnFrInclds_reportError(bma_StrBldr *pErrOrWarn, size_t pos, const char *pMessage) {
	if (pErrOrWarn != NULL) {
		if (bma_StrBldr_getSz(pErrOrWarn) != 0u) bma_StrBldr_appndChr(pErrOrWarn, '\n', 1u);
		bma_StrBldr_appndStr(pErrOrWarn, "Include scan error at offset ");
		bma_StrBldr_appndUnsgnd(pErrOrWarn, pos);
		bma_StrBldr_appndStr(pErrOrWarn, ": ");
		bma_StrBldr_appndStr(pErrOrWarn, pMessage);
	}
	return BMA_FALSE;
}

BMA_DEF bma_bool_t bma_scnFrInclds_reportWarning(bma_StrBldr *pErrOrWarn, size_t pos, const char *pMessage) {
	if (pErrOrWarn != NULL) {
		if (bma_StrBldr_getSz(pErrOrWarn) != 0u) bma_StrBldr_appndChr(pErrOrWarn, '\n', 1u);
		bma_StrBldr_appndStr(pErrOrWarn, "Include scan warning at offset ");
		bma_StrBldr_appndUnsgnd(pErrOrWarn, pos);
		bma_StrBldr_appndStr(pErrOrWarn, ": ");
		bma_StrBldr_appndStr(pErrOrWarn, pMessage);
	}
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_scnFrInclds_addInclude(
	const char *pText, size_t textLen, size_t pos, bma_IncldStmtType type,
	bma_bool_t isCndtnl, bma_ScnndIncldStmtVec *pOut, bma_StrBldr *pErrOrWarn
) {
	size_t pathStart;
	size_t cursor;
	char terminator;
	char *pPath;
	bma_ScnndIncldStmt stmt;
	bma_IMemAlloc *pAlloc = bma_Vec_getAlloc(&pOut->impl);
	size_t operandStart = pos;
	if (!bma_scnFrInclds_skipDirectiveTrivia(pText, textLen, &pos) || pos >= textLen)
		return bma_scnFrInclds_reportError(pErrOrWarn, operandStart, "missing #include operand");
	if (pText[pos] == '"') {
		terminator = '"';
		type = BMA_SCNND_INCLD_STMT_TYPE_LOCAL_FIRST;
	} else if (pText[pos] == '<') {
		terminator = '>';
		type = BMA_SCNND_INCLD_STMT_TYPE_SYS_FIRST;
	} else {
		return bma_scnFrInclds_reportWarning(pErrOrWarn, pos, "macro-form #include operand was skipped");
	}
	pathStart = ++pos;
	cursor = pos;
	while (cursor < textLen) {
		size_t spliceLen = bma_scnFrInclds_spliceLen(pText, textLen, cursor);
		if (spliceLen != 0u) {
			cursor += spliceLen;
			continue;
		}
		if (bma_scnFrInclds_newlineLen(pText, textLen, cursor) != 0u)
			return bma_scnFrInclds_reportError(pErrOrWarn, pathStart, "unterminated #include header name");
		if (pText[cursor] == terminator) break;
		++cursor;
	}
	if (cursor >= textLen)
		return bma_scnFrInclds_reportError(pErrOrWarn, pathStart, "unterminated #include header name");
	pPath = bma_strndup_ext(pText + pathStart, cursor - pathStart, pAlloc);
	if (cursor > pathStart) {
		size_t readPos = 0u;
		size_t writePos = 0u;
		while (readPos < cursor - pathStart) {
			size_t spliceLen = bma_scnFrInclds_spliceLen(pPath, cursor - pathStart, readPos);
			if (spliceLen != 0u) readPos += spliceLen;
			else pPath[writePos++] = pPath[readPos++];
		}
		pPath[writePos] = '\0';
	}
	stmt.type = type;
	stmt.pPath = pPath;
	stmt.isCndtnl = isCndtnl;
	bma_ScnndIncldStmtVec_appnd(pOut, &stmt);
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_scnFrInclds_ext(
	const char* pFileContent, size_t fileContentLen, bma_ScnndIncldStmtVec *pOut, bma_StrBldr *pErrOrWarn
) {
	size_t pos = 0u;
	size_t constructStart = 0u;
	size_t conditionalDepth = 0u;
	size_t rawDelimiterStart = 0u;
	size_t rawDelimiterLen = 0u;
	size_t guardNameStart = 0u;
	size_t guardNameLen = 0u;
	bma_bool_t guardCandidate = BMA_FALSE;
	bma_bool_t guardActive = BMA_FALSE;
	bma_bool_t guardElse = BMA_FALSE;
	bma_bool_t mayBeHeaderGuard = BMA_TRUE;
	bma_bool_t lineCanStartDirective = BMA_TRUE;
	bma_scnFrInclds_ParseState state = BMA_SCN_FR_INCLDS_STATE_NORMAL;
	char quote = '\0';
	bma_assert(pFileContent != NULL);
	bma_assert(pOut != NULL);
	while (pos < fileContentLen) {
		size_t newlineLen;
		size_t spliceLen;
		char c = pFileContent[pos];
		if (state == BMA_SCN_FR_INCLDS_STATE_LINE_COMMENT) {
			spliceLen = bma_scnFrInclds_spliceLen(pFileContent, fileContentLen, pos);
			if (spliceLen != 0u) pos += spliceLen;
			else if ((newlineLen = bma_scnFrInclds_newlineLen(pFileContent, fileContentLen, pos)) != 0u) {
				pos += newlineLen;
				state = BMA_SCN_FR_INCLDS_STATE_NORMAL;
				lineCanStartDirective = BMA_TRUE;
			} else ++pos;
			continue;
		}
		if (state == BMA_SCN_FR_INCLDS_STATE_BLOCK_COMMENT) {
			if (pos + 1u < fileContentLen && c == '*' && pFileContent[pos + 1u] == '/') {
				pos += 2u;
				state = BMA_SCN_FR_INCLDS_STATE_NORMAL;
			} else if ((newlineLen = bma_scnFrInclds_newlineLen(pFileContent, fileContentLen, pos)) != 0u) {
				pos += newlineLen;
				lineCanStartDirective = BMA_TRUE;
			} else ++pos;
			continue;
		}
		if (state == BMA_SCN_FR_INCLDS_STATE_STRING || state == BMA_SCN_FR_INCLDS_STATE_CHAR) {
			spliceLen = bma_scnFrInclds_spliceLen(pFileContent, fileContentLen, pos);
			if (spliceLen != 0u) pos += spliceLen;
			else if (c == '\\' && pos + 1u < fileContentLen) pos += 2u;
			else if ((newlineLen = bma_scnFrInclds_newlineLen(pFileContent, fileContentLen, pos)) != 0u) {
				return bma_scnFrInclds_reportError(pErrOrWarn, constructStart,
					state == BMA_SCN_FR_INCLDS_STATE_STRING ? "unterminated string literal" : "unterminated character literal");
			} else if (c == quote) {
				++pos;
				state = BMA_SCN_FR_INCLDS_STATE_NORMAL;
			} else ++pos;
			continue;
		}
		if (state == BMA_SCN_FR_INCLDS_STATE_RAW_STRING) {
			if (c == ')' && pos + rawDelimiterLen + 1u < fileContentLen &&
			    memcmp(pFileContent + pos + 1u, pFileContent + rawDelimiterStart, rawDelimiterLen) == 0 &&
			    pFileContent[pos + rawDelimiterLen + 1u] == '"') {
				pos += rawDelimiterLen + 2u;
				state = BMA_SCN_FR_INCLDS_STATE_NORMAL;
			} else ++pos;
			continue;
		}

		spliceLen = bma_scnFrInclds_spliceLen(pFileContent, fileContentLen, pos);
		if (spliceLen != 0u) {
			pos += spliceLen;
			continue;
		}
		if ((newlineLen = bma_scnFrInclds_newlineLen(pFileContent, fileContentLen, pos)) != 0u) {
			pos += newlineLen;
			lineCanStartDirective = BMA_TRUE;
			continue;
		}
		if (pos + 1u < fileContentLen && c == '/' && pFileContent[pos + 1u] == '/') {
			pos += 2u;
			state = BMA_SCN_FR_INCLDS_STATE_LINE_COMMENT;
			continue;
		}
		if (pos + 1u < fileContentLen && c == '/' && pFileContent[pos + 1u] == '*') {
			constructStart = pos;
			pos += 2u;
			state = BMA_SCN_FR_INCLDS_STATE_BLOCK_COMMENT;
			continue;
		}
		if (c == ' ' || c == '\t' || c == '\v' || c == '\f') {
			++pos;
			continue;
		}
		if (c == '#' && lineCanStartDirective) {
			size_t directivePos = pos + 1u;
			size_t afterWord = directivePos;
			size_t afterIfndef = directivePos;
			size_t afterIf = directivePos;
			size_t afterDefine = directivePos;
			size_t argPos;
			size_t identStart;
			size_t identLen;
			bma_bool_t isIfndef;
			bma_bool_t isIfDirective;
			bma_bool_t isDefine;
			bma_bool_t isIfKind;
			bma_bool_t isElKind;
			if (bma_scnFrInclds_skipDirectiveTrivia(pFileContent, fileContentLen, &directivePos)) {
				isIfndef = bma_scnFrInclds_matchDirectiveWord(pFileContent, fileContentLen, directivePos, "ifndef", &afterIfndef);
				isIfDirective = bma_scnFrInclds_matchDirectiveWord(pFileContent, fileContentLen, directivePos, "if", &afterIf);
				isDefine = bma_scnFrInclds_matchDirectiveWord(pFileContent, fileContentLen, directivePos, "define", &afterDefine);
				isIfKind = bma_scnFrInclds_matchDirectivePrefix(pFileContent, fileContentLen, directivePos, "if");
				isElKind = bma_scnFrInclds_matchDirectivePrefix(pFileContent, fileContentLen, directivePos, "el");
				if (guardCandidate && !guardActive) {
					argPos = afterDefine;
					if (isDefine && bma_scnFrInclds_readIdentifier(pFileContent, fileContentLen, &argPos, &identStart, &identLen) &&
					    identLen == guardNameLen && memcmp(pFileContent + identStart, pFileContent + guardNameStart, guardNameLen) == 0) {
						guardCandidate = BMA_FALSE;
						guardActive = BMA_TRUE;
						if (conditionalDepth != 0u) --conditionalDepth;
					} else guardCandidate = BMA_FALSE;
				}
				if (isIfKind && !isElKind) {
					bma_bool_t isHeaderGuard = BMA_FALSE;
					if (conditionalDepth == 0u && !guardActive && !guardCandidate && mayBeHeaderGuard) {
						if (isIfndef) {
							argPos = afterIfndef;
							isHeaderGuard = bma_scnFrInclds_readIdentifier(
								pFileContent, fileContentLen, &argPos, &guardNameStart, &guardNameLen);
						} else if (isIfDirective) {
							isHeaderGuard = bma_scnFrInclds_readNegatedDefinedMacro(
								pFileContent, fileContentLen, afterIf, &guardNameStart, &guardNameLen);
						}
					}
					if (isHeaderGuard) guardCandidate = BMA_TRUE;
					else mayBeHeaderGuard = BMA_FALSE;
					++conditionalDepth;
				} else if (bma_scnFrInclds_matchDirectiveWord(pFileContent, fileContentLen, directivePos, "endif", &afterWord)) {
					if (guardActive && conditionalDepth == 0u) {
						guardActive = BMA_FALSE;
						guardElse = BMA_FALSE;
					} else if (conditionalDepth != 0u) {
						--conditionalDepth;
						if (guardCandidate && conditionalDepth == 0u) guardCandidate = BMA_FALSE;
					}
				} else if (isElKind) {
					mayBeHeaderGuard = BMA_FALSE;
					if (guardActive && conditionalDepth == 0u) guardElse = BMA_TRUE;
				} else if (bma_scnFrInclds_matchDirectiveWord(pFileContent, fileContentLen, directivePos, "include", &afterWord)) {
					if (!bma_scnFrInclds_addInclude(pFileContent, fileContentLen, afterWord,
					                                BMA_SCNND_INCLD_STMT_TYPE_SYS_FIRST,
					                                conditionalDepth != 0u || guardElse, pOut, pErrOrWarn)) return BMA_FALSE;
				}
			}
			++pos;
			lineCanStartDirective = BMA_FALSE;
			continue;
		}

		if (c == 'R' && pos + 1u < fileContentLen && pFileContent[pos + 1u] == '"') {
			size_t delimStart = pos + 2u;
			size_t delimEnd = delimStart;
			while (delimEnd < fileContentLen && pFileContent[delimEnd] != '(' &&
			       bma_scnFrInclds_newlineLen(pFileContent, fileContentLen, delimEnd) == 0u &&
			       delimEnd - delimStart <= 16u && pFileContent[delimEnd] != ')' && pFileContent[delimEnd] != '\\' &&
			       pFileContent[delimEnd] != ' ' && pFileContent[delimEnd] != '\t') ++delimEnd;
			if (delimEnd < fileContentLen && pFileContent[delimEnd] == '(' && delimEnd - delimStart <= 16u) {
				constructStart = pos;
				rawDelimiterStart = delimStart;
				rawDelimiterLen = delimEnd - delimStart;
				pos = delimEnd + 1u;
				state = BMA_SCN_FR_INCLDS_STATE_RAW_STRING;
				lineCanStartDirective = BMA_FALSE;
				continue;
			}
		}
		if (c == '"' || c == '\'') {
			constructStart = pos;
			quote = c;
			state = c == '"' ? BMA_SCN_FR_INCLDS_STATE_STRING : BMA_SCN_FR_INCLDS_STATE_CHAR;
			++pos;
			lineCanStartDirective = BMA_FALSE;
			continue;
		}
		lineCanStartDirective = BMA_FALSE;
		++pos;
	}
	if (state == BMA_SCN_FR_INCLDS_STATE_BLOCK_COMMENT)
		return bma_scnFrInclds_reportError(pErrOrWarn, constructStart, "unterminated block comment");
	if (state == BMA_SCN_FR_INCLDS_STATE_STRING)
		return bma_scnFrInclds_reportError(pErrOrWarn, constructStart, "unterminated string literal");
	if (state == BMA_SCN_FR_INCLDS_STATE_CHAR)
		return bma_scnFrInclds_reportError(pErrOrWarn, constructStart, "unterminated character literal");
	if (state == BMA_SCN_FR_INCLDS_STATE_RAW_STRING)
		return bma_scnFrInclds_reportError(pErrOrWarn, constructStart, "unterminated raw string literal");
	if (conditionalDepth != 0u)
		return bma_scnFrInclds_reportError(pErrOrWarn, fileContentLen, "unterminated conditional directive");
	return BMA_TRUE;
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_SCNINC_IMPL || BMA_IMPL */

#ifdef BMA_SCNINC_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
	extern "C" {
#endif

void bma_test_exit_fail(const char *exp, const char *file, int line) {
	fprintf(stderr, "Expectation '%s' failed (file %s line %d)\n", exp, file, line);
	fflush(stderr);
	exit(1);
}

#define BMA_EXPECT(x) \
	do { \
		if(!(x)) { \
			bma_test_exit_fail(#x, __FILE__, __LINE__); \
		} \
	} while(0)

#define BMA_TEST(f) \
	do { \
		printf("Executing '" #f "()'...\n"); \
		f(); \
	} while(0)

void test_scan_includes_lexical(void) {
	const char *text =
		"// #include <comment.h>\r"
		"  #include <system.h>\r\n"
		"/* leading comment */ #include \"local.h\"\n"
		"#if defined(FLAG)\r"
		"\t# include <conditional.h>\n"
		"#endif\r\n"
		"const char *quoted = \"#include <quoted.h>\";\n"
		"const char *raw = u8R\"tag(\n #include <raw.h>\n)tag\";\n"
		"const char *futureRaw = u32R\"tag(\n #include <future-raw.h>\n)tag\";\n"
		"#include <sp\\\r\nliced.h>\n"
		"#include HEADER\n"
		"#include_next <not-include.h>\n";
	bma_ScnndIncldStmtVec includes;
	bma_ScnndIncldStmtVec_ctor(&includes);
	bma_scnFrInclds(text, &includes);
	BMA_EXPECT(bma_ScnndIncldStmtVec_getSz(&includes) == 4u);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 0u)->pPath, "system.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 0u)->type == BMA_SCNND_INCLD_STMT_TYPE_SYS_FIRST);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 0u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 1u)->pPath, "local.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 1u)->type == BMA_SCNND_INCLD_STMT_TYPE_LOCAL_FIRST);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 2u)->pPath, "conditional.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 2u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 3u)->pPath, "spliced.h") == 0);
	bma_ScnndIncldStmtVec_dtor(&includes, NULL);
}

void test_scan_includes_macro_warning(void) {
	const char *text =
		"#include HEADER\n"
		"#include <after-macro.h>\n";
	bma_ScnndIncldStmtVec includes;
	bma_StrBldr error;
	bma_ScnndIncldStmtVec_ctor(&includes);
	bma_StrBldr_ctor(&error);
	BMA_EXPECT(bma_scnFrInclds_ext(text, strlen(text), &includes, &error));
	BMA_EXPECT(strstr(bma_StrBldr_getStr(&error), "warning") != NULL);
	BMA_EXPECT(strstr(bma_StrBldr_getStr(&error), "macro-form #include") != NULL);
	BMA_EXPECT(bma_ScnndIncldStmtVec_getSz(&includes) == 1u);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 0u)->pPath, "after-macro.h") == 0);
	bma_StrBldr_dtor(&error, NULL);
	bma_ScnndIncldStmtVec_dtor(&includes, NULL);
}

void test_scan_includes_malformed_input(void) {
	const char *unterminatedString = "const char *text = \"never closed";
	const char *unterminatedRawString = "const char *text = R\"tag(never closed";
	const char *unterminatedComment = "/* never closed";
	bma_ScnndIncldStmtVec includes;
	bma_StrBldr error;
	bma_ScnndIncldStmtVec_ctor(&includes);
	bma_StrBldr_ctor(&error);
	BMA_EXPECT(!bma_scnFrInclds_ext(unterminatedString, strlen(unterminatedString), &includes, &error));
	BMA_EXPECT(strstr(bma_StrBldr_getStr(&error), "unterminated string literal") != NULL);
	bma_StrBldr_clear(&error);
	BMA_EXPECT(!bma_scnFrInclds_ext(unterminatedRawString, strlen(unterminatedRawString), &includes, &error));
	BMA_EXPECT(strstr(bma_StrBldr_getStr(&error), "unterminated raw string literal") != NULL);
	bma_StrBldr_clear(&error);
	BMA_EXPECT(!bma_scnFrInclds_ext(unterminatedComment, strlen(unterminatedComment), &includes, &error));
	BMA_EXPECT(strstr(bma_StrBldr_getStr(&error), "unterminated block comment") != NULL);
	BMA_EXPECT(!bma_scnFrInclds_ext(unterminatedString, strlen(unterminatedString), &includes, NULL));
	bma_StrBldr_dtor(&error, NULL);
	bma_ScnndIncldStmtVec_dtor(&includes, NULL);
}

void test_scan_includes_directive_prefixes(void) {
	const char *text =
		"#ifndef PREFIX_GUARD\n"
		"#define PREFIX_GUARD\n"
		"#include <guard-body.h>\n"
		"#elfoo\n"
		"#include <guard-alternative.h>\n"
		"#endif\n"
		"#include <after-guard.h>\n"
		"#ifblablah FEATURE\n"
		"#include <if-prefix.h>\n"
		"#elifdef FEATURE\n"
		"#include <elifdef.h>\n"
		"#elifndef OTHER\n"
		"#include <elifndef.h>\n"
		"#elfoo\n"
		"#include <el-prefix.h>\n"
		"#endif\n"
		"#include <after-if.h>\n";
	bma_ScnndIncldStmtVec includes;
	bma_ScnndIncldStmtVec_ctor(&includes);
	bma_scnFrInclds(text, &includes);
	BMA_EXPECT(bma_ScnndIncldStmtVec_getSz(&includes) == 8u);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 0u)->pPath, "guard-body.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 0u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 1u)->pPath, "guard-alternative.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 1u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 2u)->pPath, "after-guard.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 2u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 3u)->pPath, "if-prefix.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 3u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 4u)->pPath, "elifdef.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 4u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 5u)->pPath, "elifndef.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 5u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 6u)->pPath, "el-prefix.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 6u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 7u)->pPath, "after-if.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 7u)->isCndtnl);
	bma_ScnndIncldStmtVec_dtor(&includes, NULL);
}

void test_scan_includes_defined_guard(void) {
	const char *text =
		"#pragma once\n"
		"#include <before-guard.h>\n"
		"#if !defined ( TEST_DEFINED_GUARD )\n"
		"#define TEST_DEFINED_GUARD\n"
		"#include <defined-guard-body.h>\n"
		"#if FEATURE\n"
		"#include <nested-defined-guard.h>\n"
		"#endif\n"
		"#else\n"
		"#include <defined-guard-else.h>\n"
		"#endif\n"
		"#include <after-defined-guard.h>\n"
		"#if !defined(OTHER_GUARD) || FEATURE\n"
		"#define OTHER_GUARD\n"
		"#include <compound-condition.h>\n"
		"#endif\n";
	bma_ScnndIncldStmtVec includes;
	bma_ScnndIncldStmtVec_ctor(&includes);
	bma_scnFrInclds(text, &includes);
	BMA_EXPECT(bma_ScnndIncldStmtVec_getSz(&includes) == 6u);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 0u)->pPath, "before-guard.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 0u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 1u)->pPath, "defined-guard-body.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 1u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 2u)->pPath, "nested-defined-guard.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 2u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 3u)->pPath, "defined-guard-else.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 3u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 4u)->pPath, "after-defined-guard.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 4u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 5u)->pPath, "compound-condition.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 5u)->isCndtnl);
	bma_ScnndIncldStmtVec_dtor(&includes, NULL);
}

void test_scan_includes_parenthesized_defined_guard(void) {
	const char *text =
		"#if (!defined(PARENTHESIZED_GUARD))\n"
		"#define PARENTHESIZED_GUARD\n"
		"#include <parenthesized-body.h>\n"
		"#else\n"
		"#include <parenthesized-else.h>\n"
		"#endif\n"
		"#include <after-parenthesized-guard.h>\n";
	bma_ScnndIncldStmtVec includes;
	bma_ScnndIncldStmtVec_ctor(&includes);
	bma_scnFrInclds(text, &includes);
	BMA_EXPECT(bma_ScnndIncldStmtVec_getSz(&includes) == 3u);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 0u)->pPath, "parenthesized-body.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 0u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 1u)->pPath, "parenthesized-else.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 1u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 2u)->pPath, "after-parenthesized-guard.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 2u)->isCndtnl);
	bma_ScnndIncldStmtVec_dtor(&includes, NULL);
}

void test_scan_includes_guard(void) {
	const char *text =
		"#ifndef TEST_GUARD\n"
		"#define TEST_GUARD\n"
		"#include <guarded.h>\n"
		"#if FEATURE\n"
		"#include <nested.h>\n"
		"#endif\n"
		"#else\n"
		"#include <guard_else.h>\n"
		"#endif\n"
		"#include <after_guard.h>\n"
		"#ifndef LATE_GUARD\n"
		"#define LATE_GUARD\n"
		"#include <late_guard.h>\n"
		"#endif\n";
	bma_ScnndIncldStmtVec includes;
	bma_ScnndIncldStmtVec_ctor(&includes);
	bma_scnFrInclds(text, &includes);
	BMA_EXPECT(bma_ScnndIncldStmtVec_getSz(&includes) == 5u);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 0u)->pPath, "guarded.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 0u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 1u)->pPath, "nested.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 1u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 2u)->pPath, "guard_else.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 2u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 3u)->pPath, "after_guard.h") == 0);
	BMA_EXPECT(!bma_ScnndIncldStmtVec_at(&includes, 3u)->isCndtnl);
	BMA_EXPECT(strcmp(bma_ScnndIncldStmtVec_at(&includes, 4u)->pPath, "late_guard.h") == 0);
	BMA_EXPECT(bma_ScnndIncldStmtVec_at(&includes, 4u)->isCndtnl);
	bma_ScnndIncldStmtVec_dtor(&includes, NULL);
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_scan_includes_lexical);
	BMA_TEST(test_scan_includes_macro_warning);
	BMA_TEST(test_scan_includes_malformed_input);
	BMA_TEST(test_scan_includes_directive_prefixes);
	BMA_TEST(test_scan_includes_defined_guard);
	BMA_TEST(test_scan_includes_parenthesized_defined_guard);
	BMA_TEST(test_scan_includes_guard);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_FS_TEST */

#endif /* BMA_FS_H_INCLUDED */
