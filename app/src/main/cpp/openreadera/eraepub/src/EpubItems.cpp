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
 * Developers: ReadEra Team (2013-2020), Tarasus (2018-2020).
 */

#include "include/EpubItems.h"

void CssStyle::parseShorthandMargin(const lString16& in)
{
    if(in.empty())
    {
        return;
    }
    lString16Collection coll;
    coll.parse(in, L" ", true);
    switch (coll.length())
    {
        case 1:
            margin_top_    = coll.at(0);
            margin_right_  = coll.at(0);
            margin_bottom_ = coll.at(0);
            margin_left_   = coll.at(0);
            break;
        case 2:
            margin_top_    = coll.at(0);
            margin_right_  = coll.at(1);
            margin_bottom_ = coll.at(0);
            margin_left_   = coll.at(1);
            break;
        case 3:
            margin_top_    = coll.at(0);
            margin_right_  = coll.at(1);
            margin_bottom_ = coll.at(2);
            margin_left_   = coll.at(1);
            break;
        case 4:
            margin_top_    = coll.at(0);
            margin_right_  = coll.at(1);
            margin_bottom_ = coll.at(2);
            margin_left_   = coll.at(3);
            break;
        default:
            CRLog::error("malformed margin css tag");
            break;
    }
}

void CssStyle::parseShorthandPadding(const lString16& in)
{
    if(in.empty())
    {
        return;
    }
    lString16Collection coll;
    coll.parse(in,L" ",true);
    switch (coll.length())
    {
        case 1:
            padding_top_    = coll.at(0);
            padding_right_  = coll.at(0);
            padding_bottom_ = coll.at(0);
            padding_left_   = coll.at(0);
            break;
        case 2:
            padding_top_    = coll.at(0);
            padding_right_  = coll.at(1);
            padding_bottom_ = coll.at(0);
            padding_left_   = coll.at(1);
            break;
        case 3:
            padding_top_    = coll.at(0);
            padding_right_  = coll.at(1);
            padding_bottom_ = coll.at(2);
            padding_left_   = coll.at(1);
            break;
        case 4:
            padding_top_    = coll.at(0);
            padding_right_  = coll.at(1);
            padding_bottom_ = coll.at(2);
            padding_left_   = coll.at(3);
            break;
        default:
            CRLog::error("malformed margin css tag");
            break;
    }
}

CssStyle::CssStyle(lString16 in)
{
    source_line_ = in;
    lString16 rest;
    for (int i = 0; i < in.length(); i++)
    {
        lChar16 curr = in.at(i);
        if(curr!='{')
        {
            continue;
        }
        else
        {
            name_ = in.substr(0,i).trimDoubleSpaces(false,false,false);
            rest = in.substr(i,in.length()-i);
            break;
        }
    }
    if(rest.empty())
    {
        //CRLog::error("rest empty");
        style_string_filtered      = lString16::empty_str;
        style_string_fonts         = lString16::empty_str;
        style_string_margins       = lString16::empty_str;
        style_string_fonts_margins = lString16::empty_str;
        style_string_no_filtering  = lString16::empty_str;
        return;
    }
    //CRLog::error("rest = %s", LCSTR(rest));
    style_string_no_filtering = name_ + rest;

    direction_ = getAttrval(rest, lString16("direction"));
    if(direction_.empty())
    {
        direction_ = getAttrval(rest, lString16("dir"));
    }
    lString16 t_a = getAttrval(rest, lString16("text-align"));
    if(!t_a.empty())
    {
        if      (t_a == lString16("left"))   { text_align_ = ta_left;}
        else if (t_a == lString16("right"))  { text_align_ = ta_right;}
        else if (t_a == lString16("center")) { text_align_ = ta_center;}
        else if (t_a == lString16("justify")){ text_align_ = ta_justify;}
        else                                     { text_align_ = ta_inherit;}
    }
    margin_top_      = getAttrval(rest, lString16("margin-top"));
    margin_bottom_   = getAttrval(rest, lString16("margin-bottom"));
    margin_left_     = getAttrval(rest, lString16("margin-left"));
    margin_right_    = getAttrval(rest, lString16("margin-right"));

    if(margin_top_.empty()    &&
       margin_bottom_.empty() &&
       margin_left_.empty()   &&
       margin_right_.empty())
    {
       lString16 m = getAttrval(rest, lString16("margin"));
       parseShorthandMargin(m);
    }

    padding_top_      = getAttrval(rest, lString16("padding-top"));
    padding_bottom_   = getAttrval(rest, lString16("padding-bottom"));
    padding_left_     = getAttrval(rest, lString16("padding-left"));
    padding_right_    = getAttrval(rest, lString16("padding-right"));

    if(padding_top_.empty()    &&
       padding_bottom_.empty() &&
       padding_left_.empty()   &&
       padding_right_.empty())
    {
        lString16 p = getAttrval(rest, lString16("padding"));
        parseShorthandPadding(p);
    }

    font_weight_     = getAttrval(rest, lString16("font-weight"));
    font_style_      = getAttrval(rest, lString16("font-style"));
    text_decoration_ = getAttrval(rest, lString16("text-decoration"));
    list_style_type_ = getAttrval(rest, lString16("list-style-type"));
    display_         = getAttrval(rest, lString16("display"));

    font_family_     = getAttrval(rest, lString16("font-family"));
    while (font_family_.pos("\'")!=-1)
    {
        font_family_.replace(L"\'", L"\"");
    }

    lString16 bg = getAttrval(rest, lString16("background"));
    bg = bg.substr(bg.pos("(") + 1);
    bg = bg.substr(0, bg.pos(")"));
    background_      = bg;

    style_string_filtered      = formatCSSstring(false, false);
    style_string_fonts         = formatCSSstring(true , false);
    style_string_margins       = formatCSSstring(false, true);
    style_string_fonts_margins = formatCSSstring(true , true);

    //CRLog::error("style string = %s", LCSTR(style_string_));
};

lString16 CssStyle::getAttrval(lString16 in, lString16 attrname)
{
    lString16 result;
    int attr_start = in.pos(attrname);
    if(attr_start == -1)
    {
        //CRLog::error("getAttrval [%s] return empty str",LCSTR(attrname));
        return result;
    }

    int attr_end = -1;
    for (int i = attr_start; i < in.length(); i++)
    {
        lChar16 curr = in.at(i);
        if (curr == ';' || curr == '}')
        {
            attr_end = i;
            break;
        }
    }

    lString16 attrstr = in.substr(attr_start, attr_end - attr_start);
    //CRLog::error("attrstr = [%s]", LCSTR(attrstr));

    if(attrstr.pos(":")!=-1)
    {
        int colonpos = attrstr.pos(":")+1;
        result = attrstr.substr(colonpos,attrstr.length()-colonpos);
    }
    //CRLog::error("getAttrval [%s] return [%s]",LCSTR(attrname),LCSTR(result.trimDoubleSpaces(false, false, false)));
    return result.trimDoubleSpaces(false, false, false);
}

lString16 CssStyle::formatCSSstring(bool fonts, bool margins)
{
    lString16 text_align;
    switch (text_align_)
    {
        case ta_left    : text_align = lString16("left");    break;
        case ta_right   : text_align = lString16("right");   break;
        case ta_center  : text_align = lString16("center");  break;
        case ta_justify : text_align = lString16("justify"); break;
        case ta_inherit : text_align = lString16::empty_str;     break;
    }

    lString16 str;
    if(!display_.empty())         { str += "display:" + display_ + "; ";}
    if(!font_weight_.empty())     { str += "font-weight:" + font_weight_ + "; ";}
    if(!font_style_.empty())      { str += "font-style:" + font_style_ + "; ";}
    if(!text_decoration_.empty()) { str += "text-decoration:" + text_decoration_ + "; ";}
    if(!list_style_type_.empty()) {
        if(list_style_type_ == "upper-latin") {list_style_type_ = "upper-alpha";}
        if(list_style_type_ == "lower-latin") {list_style_type_ = "lower-alpha";}
        str += "list-style-type:" + list_style_type_ + "; ";
    }
    if(fonts &&   !font_family_.empty())   { str += "font-family:"    + font_family_   + "; "; }
    if (margins)
    {
        if (!text_align.empty())     { str += "text-align: "    + text_align     + "; ";}
        if (!margin_top_.empty())    { str += "margin-top: "    + margin_top_    + "; ";}
        if (!margin_bottom_.empty()) { str += "margin-bottom: " + margin_bottom_ + "; ";}
        if (!margin_left_.empty())   { str += "margin-left: "   + margin_left_   + "; ";}
        if (!margin_right_.empty())  { str += "margin-right: "  + margin_right_  + "; ";}

        if (!padding_top_.empty())    { str += "padding-top: "    + padding_top_    + "; ";}
        if (!padding_bottom_.empty()) { str += "padding-bottom: " + padding_bottom_ + "; ";}
        if (!padding_left_.empty())   { str += "padding-left: "   + padding_left_   + "; ";}
        if (!padding_right_.empty())  { str += "padding-right: "  + padding_right_  + "; ";}
    }

    if(str.empty())
    {
        //CRLog::error(" str.empty() class [%s], sourceline = [%s]",LCSTR(name_),LCSTR(source_line_));
        return lString16::empty_str;
    }
    if(!name_.empty())
    {
        str = name_ + " { " + str + "}";
    }
    else
    {
        str = "{" + str + "}";
    }
    //CRLog::error("class = [%s]", LCSTR(str));
    return str;
}

CssStyle CssStyle::OverwriteClass(CssStyle additional)
{
    CssStyle base = *this;
    if (base.source_line_.empty() || additional.source_line_.empty())
    {
        return base;
    }

    if (additional.name_ != base.name_)
    {
        return base;
    }
    base.source_line_ = base.source_line_ + "\n" + additional.source_line_;

    //if (base.margin_top_!= additional.font_weight_ && !additional.margin_top_.empty())
    //{
    //    base.margin_top_ = additional.margin_top_;
    //}
    //if (base.margin_bottom_!= additional.font_weight_ && !additional.margin_bottom_.empty())
    //{
    //    base.margin_bottom_ = additional.margin_bottom_;
    //}
    //if (base.text_align_ != additional.text_align_ && additional.text_align_ > ta_left)
    //{
    //    base.text_align_ = additional.text_align_;
    //}
    if (base.font_weight_ != additional.font_weight_ && !additional.font_weight_.empty())
    {
        base.font_weight_ = additional.font_weight_;
    }
    if (base.font_style_ != additional.font_style_ && !additional.font_style_.empty())
    {
        base.font_style_ = additional.font_style_;
    }
    if (base.text_decoration_ != additional.text_decoration_ && !additional.text_decoration_.empty())
    {
        base.text_decoration_ = additional.text_decoration_;
    }
    if (base.background_ != additional.background_ && !additional.background_.empty())
    {
        base.background_ = additional.background_;
    }

    //CRLog::error("base style string was[%s]",LCSTR(base.style_string_));
    //CRLog::error("add style string was[%s]",LCSTR(additional.style_string_));

    base.style_string_filtered      = formatCSSstring(false, false);
    base.style_string_fonts         = formatCSSstring(true , false);
    base.style_string_margins       = formatCSSstring(false, true);
    base.style_string_fonts_margins = formatCSSstring(true , true);
    //CRLog::error("merged style string is[%s]",LCSTR(base.style_string_));

    return base;
}

bool EpubStylesManager::CheckClassName(lString16 name)
{
    //CRLog::error("check classname = [%s]",LCSTR(name));
    if(name.empty())
    {
        return false;
    }
    for (int i = 0; i < name.length(); i++)
    {
        lChar16 ch = name.at(i);
        if ((ch >= 45 && ch <= 57)     || //0-9
            (ch >= 65 && ch <= 90)     || //A-Z
            (ch >= 97 && ch <= 122)    || //a-z
            (ch == ' ') || (ch == '.') ||
            (ch == ',') || (ch == '=') ||
            (ch == '_') || (ch == '"') ||
            (ch == '<') || (ch == '>') ||
            (ch == '[') || (ch == ']'))
        {
            continue;
        }
        else
        {
            //CRLog::error("Found illegal character in CSS class name: [%s] -> [%lc]",LCSTR(name),ch);
            return false;
        }
    }

    lChar16 last = 0;
    int q_count = 0;
    bool br_open = false;
    for (int i = 0; i < name.length(); i++)
    {
        lChar16 ch = name.at(i);
        if (last == '.' || last == ' ' || last == ',')
        {
            if ((ch >= 45 && ch <= 57) || ch == '-')
            {
                //CRLog::error("Illegal character combination in css class name: [%s] -> [%lc][%lc]",LCSTR(name),last,ch);
                return false;
            }
        }
        if(ch == '[')
        {
            if(br_open)
            {
                //CRLog::error("brackets error 1");
                return false;
            }
            br_open = true;
        }
        else if(ch == ']')
        {
            if(!br_open)
            {
                //CRLog::error("brackets error 2");
                return false;
            }
            br_open = false;
        }
        else if(ch=='"')
        {
            q_count++;
        }

        last = ch;
    }
    if(q_count %2 != 0 || br_open)
    {
        //CRLog::error("unpaired quotes or br_open");
        return false;
    }

    return true;
}

lString16Collection EpubStylesManager::splitSelectorToClasses(lString16 selector)
{
    lString16Collection result;
    selector = selector.trimDoubleSpaces(false,false,false);
    //CRLog::error("selector in = %s",LCSTR(selector));
    if(selector.pos(" ")!=-1||selector.pos(",")!=-1)
    {
        lString16 buf;
        bool save = false;
        for (int i = 0; i < selector.length(); i++)
        {
            lChar16 curr = selector.at(i);
            if(curr == '.')
            {
                save = true;
            }
            else if(curr == ',')
            {
                result.add(buf);
                buf.clear();
                save = false;
                continue;
            }
            else if( i == selector.length()-1)
            {
                buf +=curr;
                result.add(buf);
                buf.clear();
                save = false;
                break;
            }
            if(save)
            {
                buf += curr;
            }
        }
        return result;
    }
    else
    {
        result.add(selector);
        return result;
    }
}

void EpubStylesManager::addCssRTLClass(CssStyle css)
{
    if (css.isRTL())
    {
        lString16Collection names = splitSelectorToClasses(css.name_);
        for (int i = 0; i < names.length(); i++)
        {
            lString16 name = names.at(i);
            name = (name.startsWith("."))? name.substr(1,css.name_.length()-1) : name ;
            rtl_map_.insert(std::make_pair(name.getHash(), css));
            //CRLog::error("rtl class in map = %s",LCSTR(name));
        }
    }

}

void EpubStylesManager::addCSSClass(CssStyle css , EpubCSSMap *map)
{
    addCssRTLClass(css);

    if (css.name_.empty())
    {
        //CRLog::error("name_ empty");
        return;
    }
    if( css.source_line_.empty())
    {
        //CRLog::error("source_line_ empty");
        return;
    }
    if(css.style_string_fonts.empty() && css.style_string_filtered.empty() && css.background_.empty())
    {
        //CRLog::error("style_string_ empty");
        return;
    }
    if(!CheckClassName(css.name_))
    {
        //CRLog::error("class check failed");
        return;
    }
    if(map->find(css.name_.getHash())!=map->end())
    {
        CssStyle base = map->at(css.name_.getHash());
        css = base.OverwriteClass(css);
    }
    map->operator[](css.name_.getHash()) = css;

    //CRLog::trace("EpubCSSclass added [%s] ",LCSTR(css.name_));
    //classes_array_.add(css);
}

lString16Collection EpubStylesManager::SplitToClasses(lString16 in)
{
    lString16Collection result;
    int classstart = 0;
    int classend = -1;
    bool comment_skip = false;
    for (int i = 0; i < in.length(); i++)
    {
        lChar16 curr = in.at(i);

        if(comment_skip && !(curr =='*' && in.at(i+1) == '/'))
        {
            continue;
        }
        if(curr == '*' && in.at(i+1) == '/')
        {
            comment_skip = false;
            continue;
        }
        if(curr == '/' && in.at(i+1) == '*')
        {
            comment_skip = true;
            continue;
        }

        if (curr != '}')
        {
            continue;
        }
        else
        {
            classend = i + 1;
            lString16 CSSclass = in.substr(classstart, classend - classstart);
            result.add(CSSclass);
            classstart = classend;
            classend = -1;
            continue;
        }

    }
    for (int i = 0; i < result.length(); i++)
    {
        if(result.at(i).pos("{")==-1 || result.at(i).pos("}")==-1)
        {
            result.erase(i,1);
        }
    }
    return result;
}

void EpubStylesManager::parseString(lString16 in)
{
    {
        in = in.trimDoubleSpaces(false,false,false);
        lString16Collection classes_str_coll = SplitToClasses(in);
        for (int i = 0; i < classes_str_coll.length(); i++)
        {
            CssStyle cssClass = CssStyle(classes_str_coll.at(i));
            this->addCSSClass(cssClass, &classes_map_);
        }
    }
}

void EpubStylesManager::Finalize()
{
    EpubCSSMap newMap;
    std::map<lUInt32 , CssStyle>::iterator it = classes_map_.begin();
    while (it != classes_map_.end())
    {
        //lUInt32 hash = it->first; //key
        CssStyle curr = it->second; //value

        all_classes_filtered.append(curr.style_string_filtered);
        all_classes_filtered.append("\n");

        all_classes_fonts.append(curr.style_string_fonts);
        all_classes_fonts.append("\n");

        all_classes_margins.append(curr.style_string_margins);
        all_classes_margins.append("\n");

        all_classes_fonts_margins.append(curr.style_string_fonts_margins);
        all_classes_fonts_margins.append("\n");

        all_classes_not_filtered.append(curr.style_string_no_filtering);
        all_classes_not_filtered.append("\n");

        lString16 className = curr.name_;
        while (className.pos(L".")!=-1)
        {
            className = className.substr(className.pos(L".")+1);
        }
        curr.name_ = className;
        addCSSClass(curr,&newMap);
        it++;
    }
    //classes_map_.clear();
    classes_map_ = newMap;
    //classes_map_.insert(newMap.begin(), newMap.end());
}

bool EpubStylesManager::ClassIsRTL(lString16 name) //array scan
{
    if(rtl_map_.empty())
    {
        return false;
    }
    if(rtl_map_.find(name.getHash())!=rtl_map_.end())
    {
        return true;
    }
    return false;
}

lString16 EpubStylesManager::as_string()
{
    switch (gEmbeddedStylesLVL)
    {
        case 1: return all_classes_filtered;
        case 2: return all_classes_fonts;
        case 3: return all_classes_margins;
        case 4: return all_classes_fonts_margins;
        case 5: return all_classes_not_filtered;
        default: CRLog::error("embedded styles level = 0!");
            return lString16::empty_str;
    }
}

bool EpubStylesManager::classExists(lString16 className)
{
    if(classes_map_.empty())
    {
        return false;
    }
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return true;
    }
    return false;
}

CssStyle EpubStylesManager::getClass(lString16 className)
{
    if(classes_map_.empty())
    {
        return CssStyle();
    }
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return classes_map_.at(className.getHash());
    }
    return CssStyle();
}

bool EpubStylesManager::classIsBold(lString16 className)
{
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return classes_map_[className.getHash()].isBold();
    }
    return false;
}

bool EpubStylesManager::classIsItalic(lString16 className)
{
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return classes_map_[className.getHash()].isItalic();
    }
    return false;
}

bool EpubStylesManager::classIsUnderline(lString16 className)
{
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return classes_map_[className.getHash()].isUnderline();
    }
    return false;
}
