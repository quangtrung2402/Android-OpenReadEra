/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020),
 * Tarasus (2018-2020).
 */

#include <eraepub/include/devanagariManager.h>
#include "EraEpubBridge.h"

void CreBridge::processTextSearchPreviews(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_PREVIEWS;
    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processTextSearchPreviews bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 inputstr = lString16(val);

    if(inputstr.empty())
    {
        CRLog::error("processTextSearchPreviews bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    lString16 query  = inputstr;
    int pagestart = 0;
    int pageend = doc_view_->GetPagesCount()-1;

    int sep = inputstr.pos(":");
    if (sep > 0)
    {
        lString16 pages_s = inputstr.substr(0, sep);
        lString16Collection pages_c;
        pages_c.parse(pages_s, L"-", false);
        pagestart = atoi(LCSTR(pages_c.at(0)));
        pageend   = atoi(LCSTR(pages_c.at(1)));
        query = inputstr.substr(sep + 1, inputstr.length() - sep + 1);
    }

    if (query.empty())
    {
        CRLog::error("processTextSearchPreviews bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    int slash_n = query.pos("\n");
    while (slash_n!=-1)
    {
        query.replace(slash_n,1,lString16(" "));
        slash_n = query.pos("\n");
    }
    if(DVNG_DISPLAY_ENABLE == 1 && gDocumentDvng == 1)
    {
        query = lString16::processDvngText(query);
    }
    if(BANGLA_DISPLAY_ENABLE == 1 && gDocumentBangla == 1)
    {
        query = lString16::processBanglaText(query);
    }
    for (int p = pagestart; p <= pageend; p ++)
    {
        auto page = (uint32_t) ImportPage(p, doc_view_->GetColumns());
        LVArray<SearchResult> searchPreviews = doc_view_->SearchForTextPreviews(page, query);

        for (int i = 0; i < searchPreviews.length(); i++)
        {
            SearchResult curr = searchPreviews.get(i);

            lString16 text = curr.preview_;
            while (text.pos(L"\n")!=-1)
            {
                text.replace(L"\n", L" ");
            }
            //text.trimDoubleSpaces(false,false,false);

            lString16 xpointers_str;
            for (int j = 0; j < curr.xpointers_.length(); j++)
            {
                xpointers_str += curr.xpointers_.get(j);
                xpointers_str += ";";
            }
            xpointers_str = xpointers_str.substr(0, xpointers_str.length() - 1);

            //CRLog::error("to response:");
            //CRLog::error("Page      : %d", page);
            //CRLog::error("xplen     : %d", curr.xpointers_.length());
            //CRLog::error("Xpointers : %s", LCSTR(xpointers_str));
            //CRLog::error("Preview   : %s", LCSTR(text));
            response.addInt(p);
            responseAddString(response, xpointers_str); //xpaths
            responseAddString(response, lString16::restoreIndicText(text));
        }
    }
}

void CreBridge::processTextSearchHitboxes(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_HITBOXES;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processTextSearchHitboxes bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 inputstr = lString16(val);
    //lString16 inputstr = lString16("3:text"); //page:query

    if(inputstr.empty())
    {
        CRLog::error("processTextSearchHitboxes bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    lString16 query;
    uint32_t external_page;
    int sep = inputstr.pos(":");
    if (sep > 0)
    {
        lString16 pages_s = inputstr.substr(0, sep);
        external_page = atoi(LCSTR(pages_s));
        query = inputstr.substr(sep + 1, inputstr.length() - sep + 1);
    }

    if (query.empty())
    {
        CRLog::error("processTextSearchPreviews bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    int slash_n = query.pos("\n");
    while (slash_n!=-1)
    {
        query.replace(slash_n,1,lString16::empty_str);
        slash_n = query.pos("\n");
    }

    auto page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());

    if(DVNG_DISPLAY_ENABLE == 1 && gDocumentDvng == 1)
    {
        query = lString16::processDvngText(query);
    }
    if(BANGLA_DISPLAY_ENABLE == 1 && gDocumentBangla == 1)
    {
        query = lString16::processBanglaText(query);
    }
    if(!query.empty())
    {
        LVArray<Hitbox> hitboxes = doc_view_->SearchForTextHitboxes(page,query);
        for (int i = 0; i < hitboxes.length(); i++)
        {
            Hitbox curr = hitboxes.get(i);
            if (RTL_DISPLAY_ENABLE && gDocumentRTL == 1)// && node->isRTL())
            {
                float offset = 0;

                LVFont * font = curr.word_.getNode()->getParentNode()->getFont().get();
                if(gFlgFloatingPunctuationEnabled)
                {
                    offset = font->getVisualAligmentWidth()/2;
                    offset = offset / doc_view_->GetWidth();
                }

                curr.left_ = curr.left_ - 0.5f;
                curr.left_ = curr.left_ * -1;
                curr.left_ = curr.left_ + 0.5f + offset;

                curr.right_ = curr.right_ - 0.5f;
                curr.right_ = curr.right_ * -1;
                curr.right_ = curr.right_ + 0.5f + offset;

                hitboxes.set(i,curr);
            }
        }
        for (int i = 0; i < hitboxes.length(); i++)
        {
            Hitbox currHitbox = hitboxes.get(i);
            response.addFloat(currHitbox.left_);
            response.addFloat(currHitbox.top_);
            response.addFloat(currHitbox.right_);
            response.addFloat(currHitbox.bottom_);
            responseAddString(response, lString16::restoreIndicText(currHitbox.text_));
        }
    }
}

void CreBridge::processTextSearchGetSuggestionsIndex(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_INDEXER;
    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processTextSearchGetSuggestionsIndex bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 inputstr = lString16(val);

    if (inputstr.empty())
    {
        CRLog::error("processTextSearchGetSuggestionsIndex bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    lString16Collection pages_c;
    pages_c.parse(inputstr, L"-", false);
    int pagestart = atoi(LCSTR(pages_c.at(0)));
    int pageend = atoi(LCSTR(pages_c.at(1)));

    //lString16Map map = doc_view_->GetWordsIndexesMap();

    for (int page_index = pagestart; page_index < pageend; page_index++)
    {
        lString16Map map = doc_view_->GetPhrasesIndexesMapForPage(page_index);
        for (lString16Map::iterator it = map.begin(); it != map.end(); it++)
        {
            responseAddString(response, lString16(it->first.c_str()));
            response.addInt(it->second);
        }
    }
}
