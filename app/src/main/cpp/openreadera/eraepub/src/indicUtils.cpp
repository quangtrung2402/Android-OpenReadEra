//
// Created by Tarasus on 08.09.2020.
// moved several indic script-related methods from lvstring.cpp to avoid code bloating
//
#include <eraepub/include/devanagariManager.h>
#include <eraepub/include/banglaManager.h>
#include "include/lvstring.h"


lString16 lString16::restoreIndicText(lString16 str)
{
    lString16 res = str;
    if (DVNG_DISPLAY_ENABLE && gDocumentDvng)
    {
        lString16Collection words;
        words.parse(res, ' ', false);
        res.clear();
        for (int i = 0; i < words.length(); ++i)
        {
            lString16 curr = words.at(i);
            curr = restoreDvngWord(curr);
            res += curr + " ";
        }
        res = res.substr(0, res.length() - 1);
    }
    if (BANGLA_DISPLAY_ENABLE && gDocumentBangla)
    {
        lString16Collection words;
        words.parse(res, ' ', false);
        res.clear();
        for (int i = 0; i < words.length(); ++i)
        {
            lString16 curr = words.at(i);
            curr = restoreBanglaWord(curr);
            res += curr + " ";
        }
        res = res.substr(0, res.length() - 1);
    }
    return res;
}

//DEVANAGARI ZONE

bool lString16::CharIsDvng(int ch)
{
    return ((ch >= 0x0900 && ch <= 0x097F) || (ch >= 0xA8E0 && ch <= 0xA8FF) || (ch >= 0x1CD0 && ch <= 0x1CFA));
}

bool isViramaComboDvng(lChar16 ch)
{
    return (ch == 0xE030 ||ch == 0xE031||
            ch == 0xE032 || ch == 0xE033 || ch == 0xE034 || ch == 0xE035 ||
            ch == 0xE036 || ch == 0xE037 || ch == 0xE038 || ch == 0xE039 ||
            ch == 0xE03A || ch == 0xE03B || ch == 0xE03C || ch == 0xE03D ||
            ch == 0xE03E || ch == 0xE03F || ch == 0xE040 || ch == 0xE041 ||
            ch == 0xE042 || ch == 0xE043 || ch == 0xE044 || ch == 0xE045 ||
            ch == 0xE046 || ch == 0xE047 || ch == 0xE048 || ch == 0xE049 ||
            ch == 0xE04B || ch == 0xE04C || ch == 0xE04D || ch == 0xE04E ||
            ch == 0xE04F || ch == 0xE050 || ch == 0xE051 || ch == 0xE052 ||
            ch == 0xE053 || ch == 0xE056 || ch == 0xE057 || ch == 0xE058 ||
            ch == 0xE05D || ch == 0xE062 || ch == 0xE063 || ch == 0xE069 ||
            ch == 0xE06B || ch == 0xE06F || ch == 0xE071 || ch == 0xE194 );

}

bool isDevanagari_consonant(lChar16 ch)
{
    if(ch == 0x0929 || ch == 0x0931 || ch == 0x0933 || ch == 0x0934 ) return false;
    if(ch >= 0x0915 && ch <= 0x0939) return true;
    return false;
}

bool lString16::CheckDvng()
{
    if(gDocumentDvng == 1)
    {
        return true;
    }
    for (int i = 0; i < this->length(); i+=10)
    {
        int ch = this->at(i);
        if (CharIsDvng(ch))
        {
            gDocumentDvng = 1;
            if(gDvngLigMapRev.empty())
            {
                gDvngLigMapRev = DevanagariLigMapReversed();
            }
            return true;
        }
    }
    return false;
}

void lString16::SwitchDvngI(lString16* str)
{
/*
 *  (? ` i) -> (i ? `)
 *  (? i)   -> (i ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x93f)
        {
            if ( i>=2 && isViramaComboDvng(str->at(i - 2)) )
            {
                //replace  "I" with 0xE1C6, and move three (two, because ligature) symbols left
                str->at(i)     = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x93f;
            }
            else if (i >= 2 && str->at(i - 1) == 0xE02E) //ra reph form + i fix
            {
                //LE("ra reph form + i");
                str->at(i)     = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x93f;
            }
            else
            {
                //LE("i regular");
                str->at(i)     = str->at(i - 1);
                str->at(i - 1) = 0x93f;
            }
        }
    }
}

void lString16::SwitchDvngReph_reverse(lString16* str)
{
/*
 *  (i ? `) -> (i ` ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0xE02E)
        {
            str->at(i) = str->at(i - 1);
            str->at(i - 1) = 0xE02E;
        }
    }
}

void lString16::SwitchDvngI_reverse(lString16* str)
{
/*
 *  (i ` ?) -> (` ? i)
 *  (i ?)   -> (? i)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x093F)
        {
            if( i <= str->length()-3 && isViramaComboDvng(str->at(i + 1)))
            {
                str->at(i) = str->at(i+1);
                str->at(i+1) = str->at(i+2);
                str->at(i+2) = 0x093F;
            }
            else if( i <= str->length()-3 && str->at(i+1)== 0xE02E)
            {
                str->at(i) = str->at(i+1);
                str->at(i+1) = str->at(i+2);
                str->at(i+2) = 0x093F;
            }
            else
            {
                str->at(i) = str->at(i+1);
                str->at(i+1) = 0x093f;
            }
        }
    }
}

lString16 processDvngLigatures(lString16 word)
{
    // devanagari lig max length = 6
    int j = (word.length() >= 6 ) ? 6 : word.length();
    for(;  j >= 2; j--)
    {
        for (int c = word.length()-j; c >=0  ; c --)
        {
            //lUInt32 fastkey = (word.at(c) << 16) + j;
            lUInt32 fastkey = (word.at(c) << 16) + word.at(c+1);
            if(gDvngFastLigMap.find(fastkey) == gDvngFastLigMap.end())
            {
                continue;
            }

            dvngLig lig(word.substr(c,j));
            lChar16 rep = findDvngLigRev(lig);

            //ra 0930 eyelash (ra + virama + ZWNJ + consonant)
            if(rep == 0xE04A )
            {
                if(isDevanagari_consonant(word[c+j]))
                {
                    word.replace(c,j,lString16(&rep,1));
                    c-=j-2;
                }
            }
            else if(rep == 0xE02E )
            {
                word.replace(c,j,lString16(&rep,1));
                word[c] = word[c+1];
                word[c+1] = rep;
                c-=j-2;
            }
            else if(rep!=0)
            {
                //LE("found!!, char = 0x%X",rep);
                word.replace(c,j,lString16(&rep,1));
                c-=j-2;
            }
        }
    }
    return word;
}

lString16 lString16::processDvngText(lString16 str)
{
    if(str.length() < 2)
    {
        return str;
    }
    lString16 res;
    lString16Collection words;

    words.parse(str,' ',true);
    for (int i = 0; i < words.length(); i++)
    {
        lString16 word = words.at(i);

        if(word.length()<2)
        {
            res.append(word);
            res.append(L" ");
            continue;
        }
        word = processDvngLigatures(word);
        SwitchDvngI(&word);

        res.append(word);
        res.append(L" ");
    }
    res.substr(0,res.size()-1);
    return res;
}


//END OF DEVANAGARI ZONE

//BANGLA ZONE

bool isViramaComboBangla(lChar16 ch)
{
    return (ch == 0xE225 ||     //Ra1 virama (reph)
            ch == 0xE226 ||     //Ra  virama (reph)
            ch == 0xE24C ||     //Ra1 virama (full)
            ch == 0xE266);      //Ra  virama (full)

}

bool isBangla_ligature(lChar16 ch)
{
    return (ch>= BANGLA_START && ch <= BANGLA_END);
}

bool isBangla_consonant(lChar16 ch)
{
    if(ch >= 0x0995 && ch <= 0x09B9) return true;
    if(ch >= 0xE204 && ch <= 0xE206) return true;
    return false;
}

void lString16::SwitchBanglaI(lString16* str)
{
/*
* (E ? i) -> (i E ?)  // E = 0x09C7
* (? J i) -> (i ? J)  // J = 0xE225
* (? j i) -> (i ? j)  // j = 0xE226
* (? x i) -> (i ? x)  // x = 0xE4EA #509 "0x09BF 0x09B7 0x09CD 0x099F" //0x1019D
* (? Y i) -> (i ? Y)  // Y = 0xE271 ya postform
* (? y i) -> (i ? y)  // y = 0xE272 ya postform
* (? U i) -> (i ? U)  // U = 0xE273 ya postform
* (? i)   -> (i ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x09BF)
        {
            if (i >= 2 && str->at(i - 2) == 0x09C7) //fix for bangla E vovel
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x09C7;
                str->at(i - 2) = 0x09BF;
            }
            else if (i >= 2 && (str->at(i - 1) == 0xE225 ||
                                str->at(i - 1) == 0xE226 ||
                                str->at(i - 1) == 0xE4EA ||
                                str->at(i - 1) == 0xE271 ||
                                str->at(i - 1) == 0xE272 ||
                                str->at(i - 1) == 0xE273)  )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x9Bf;
            }
            else
            {
                //LE("i 0x9BF regular");
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x9BF;
            }
        }
    }
}

void lString16::SwitchBanglaReph(lString16* str)
{
/*
 *
 *  (? E)   -> (E ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 0; i < str->length()-1; i++)
    {
        if (str->at(i) == 0xE225)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0xE225;
            i++;
        }
        if (str->at(i) == 0xE226)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0xE226;
            i++;
        }
    }
}

void lString16::SwitchBanglaE(lString16* str)
{
/*    ( ? 0xE271 E)   -> ( E ? 0xE271)
 *    ( ? 0xE272 E)   -> ( E ? 0xE272)
 *    ( ? 0xE273 E)   -> ( E ? 0xE273)
 *    ( ? E )         -> ( E ? )
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x09C7)
        {
            if( str->at(i - 1) == 0xE271 ||  //ya postform
                str->at(i - 1) == 0xE272 ||  //ya postform
                str->at(i - 1) == 0xE273 ||  //ya postform
                str->at(i - 1) == 0xE225 ||  //reph
                str->at(i - 1) == 0xE226   ) //reph
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x09C7;
            }
            else
            {
                //default
                str->at(i + 0) = str->at(i - 1);
                str->at(i - 1) = 0x09C7;
            }
        }
    }
}

void lString16::SwitchBanglaAI(lString16* str)
{
/*
 *  (? E)   -> (E ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x09C8)
        {
            str->at(i) = str->at(i - 1);
            str->at(i - 1) = 0x09C8;
        }
    }
}

void lString16::SwitchBanglaO(lString16 *str)
{
/*
 * (? O)   -> (e ? aa)
 * (? ' O) -> (e ? ' aa) // ' == 0xE226 //ra virama
 * (? J O) -> (e ? J aa) // J == 0xE271 //ya postform
 * (? j O) -> (e ? j aa) // j == 0xE272 //ya postform
 * (? i O) -> (e ? i aa) // i == 0xE273 //ya postform
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x09CB)
        {
            if (i > 1 && (str->at(i - 1) == 0xE226 ||
                          str->at(i - 1) == 0xE271 ||
                          str->at(i - 1) == 0xE272 ||
                          str->at(i - 1) == 0xE273) )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x09C7;
                str->insert(i + 1, 1, 0x09BE);
            }
            else
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x09C7;
                str->insert(i + 1, 1, 0x09BE);
            }
        }
    }
}

void lString16::SwitchBanglaOU(lString16 *str)
{
/*
 * (? OU)   -> (e ? ou)
 * (? ' OU) -> (e ? ' ou) // ' == 0xE226 //ra virama
 * (? J OU) -> (e ? J ou) // J == 0xE271 //ya postform
 * (? j OU) -> (e ? j ou) // j == 0xE272 //ya postform
 * (? i OU) -> (e ? i ou) // i == 0xE273 //ya postform
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x09CC)
        {
            if (i > 1 && (str->at(i - 1) == 0xE226 ||
                          str->at(i - 1) == 0xE271 ||
                          str->at(i - 1) == 0xE272 ||
                          str->at(i - 1) == 0xE273) )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x09C7;
                str->insert(i + 1, 1, 0x09D7);
            }
            else
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x09C7;
                str->insert(i + 1, 1, 0x09D7);
            }
        }
    }
}

void lString16::SwitchBanglaRaITa(lString16 *str)
{
    //0x09B0 0x09bf 0x09a4 -> 0x09BF 0x09A4 0xE226
    if (str->length() < 3)
    {
        return;
    }
    for (int i = 0; i < str->length()-2; i++)
    {
        if (str->at(i + 0) == 0x09B0 && str->at(i + 1) == 0x09BF && str->at(i + 2) == 0x09A4)
        {
            str->at(i + 0) = 0x09BF;
            str->at(i + 1) = 0x09A4;
            str->at(i + 2) = 0xE226;
        }
    }
}

void lString16::SwitchBanglaI_reverse(lString16* str)
{

/*
 * (i E ?) -> (E ? i)
 * (i ? J) -> (? J i) // J = 0xE225
 * (i ? j) -> (? j i) // j = 0xE226
 * (i ? x) -> (? j x) // x = 0xE4EA
 * (i ? Y) -> (? Y i) // Y = 0xE271 ya postform
 * (i ? y) -> (? y i) // y = 0xE272 ya postform
 * (i ? U) -> (? U i) // U = 0xE273 ya postform
 * (i ?)   -> (? i)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if (str->at(i) == 0x09BF)
        {
            if (str->at(i + 1) == 0x09C7) //fix for bangla E vovel
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x09BF;
            }
            else if (str->at(i+2) == 0xE225 ||
                     str->at(i+2) == 0xE226 ||
                     str->at(i+2) == 0xE4EA ||
                     str->at(i+2) == 0xE271 ||
                     str->at(i+2) == 0xE272 ||
                     str->at(i+2) == 0xE273   )
            {
                str->at(i + 0) = str->at(i+1);
                str->at(i + 1) = str->at(i+2);
                str->at(i + 2) = 0x09BF;
            }
            else
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x09Bf;
            }
        }
    }
}

void lString16::SwitchBanglaE_reverse(lString16* str)
{
/*  ( E ? 0xE271) -> ( ? 0xE271 E )
 *  ( E ? 0xE272) -> ( ? 0xE272 E )
 *  ( E ? 0xE273) -> ( ? 0xE273 E )
 *  ( E ? )       -> ( ? E )
 */
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2 ; i >= 0 ; i--)
    {
        if (str->at(i) == 0x09C7)
        {
            if( i <= str->length()-3 &&
                ( str->at(i + 2) == 0xE271 ||  //ya postform
                  str->at(i + 2) == 0xE272 ||  //ya postform
                  str->at(i + 2) == 0xE273 ||  //ya postform
                  str->at(i + 2) == 0xE225 ||  //reph
                  str->at(i + 2) == 0xE226   ) //reph
               )
            {
                str->at(i + 0) = str->at(i+1);
                str->at(i + 1) = str->at(i+2);
                str->at(i + 2) = 0x09C7;
                i+=2;
            }
            else
            {
                //default
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = 0x09C7;
                i++;
            }
        }
    }
}

void lString16::SwitchBanglaAI_reverse(lString16* str)
{
/*
 *  (e ?)   -> (? e)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x09C8)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x09C8;
        }
    }
}

void lString16::SwitchBanglaO_reverse(lString16* str)
{
/*
 *  (e ? aa)   -> (? O)
 *  (e ? ' aa) -> (? ' O)  // ' == 0xE226 //ra virama
 *  (e ? J aa) -> (? J O)  // J == 0xE271 //ya postform
 *  (e ? j aa) -> (? j O)  // j == 0xE272 //ya postform
 *  (e ? i aa) -> (? i O)  // i == 0xE273 //ya postform
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x09C7)
        {
            if ( i < str->length()-3    &&
                 str->at(i+3) == 0x09BE && (str->at(i + 2) == 0xE226 ||
                                            str->at(i + 2) == 0xE271 ||
                                            str->at(i + 2) == 0xE272 ||
                                            str->at(i + 2) == 0xE273 ))
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x09CB;
                str->erase(i + 3);
                continue;
            }
            if (str->at(i + 2) == 0x09BE)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x09CB;
                str->erase(i + 2);
            }
        }
    }
}

void lString16::SwitchBanglaOU_reverse(lString16* str)
{
/*
 *  (e ? ' ou) -> (? ' OU)  // ' == 0xE226 //ra virama
 *  (e ? J ou) -> (? J OU)  // J == 0xE271 //ya postform
 *  (e ? j ou) -> (? j OU)  // j == 0xE272 //ya postform
 *  (e ? i ou) -> (? i OU)  // i == 0xE273 //ya postform
 *  (e ? ou)   -> (? OU)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x09C7)
        {
            if ( i < str->length()-3    &&
                 str->at(i+3) == 0x09D7 && (str->at(i + 2) == 0xE226 ||
                                            str->at(i + 2) == 0xE271 ||
                                            str->at(i + 2) == 0xE272 ||
                                            str->at(i + 2) == 0xE273 ))
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x09CC;
                str->erase(i + 3);
                continue;
            }
            if (str->at(i + 2) == 0x09D7)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x09CC;
                str->erase(i + 2);
            }
        }
    }
}

void lString16::SwitchBanglaRaITa_reverse(lString16* str)
{
    // 0x09BF 0x09A4 0xE226 -> 0x09B0 0x09BF 0x09A4
    if (str->length() < 3)
    {
        return;
    }
    for (int i = 0; i < str->length()-2; i++)
    {
        if (str->at(i + 0) == 0x09BF && str->at(i + 1) == 0x09A4 && str->at(i + 2) == 0xE226)
        {
            str->at(i + 0) = 0x09B0;
            str->at(i + 1) = 0x09BF;
            str->at(i + 2) = 0x09A4;
        }
    }
}

void lString16::SwitchBanglaReph_reverse(lString16* str)
{
/*
 *  (? e `) -> (` ? e) // e = 0x09C7
 *  (? i `) -> (` ? i) // i = 0x09bf
 *  (? `)   -> (` ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = str->length() - 1; i >=1 ; i--)
    {
        int ch = str->at(i);
        if (ch == 0xE266 || ch == 0xE24C || ch ==  0xE226 || ch ==  0xE225)
        {
            if( i >= 2 && (str->at(i - 1) == 0x09C7 || str->at(i - 1) == 0x09BF) )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = ch;
                i-=2;
            }
            else
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = ch;
                i--;
            }
        }
    }
}

bool lString16::CharIsBangla(int ch)
{
    return ((ch >= 0x0980 && ch <= 0x09FF));
}

bool lString16::CheckBangla()
{
    if(gDocumentBangla == 1)
    {
        return true;
    }
    for (int i = 0; i < this->length(); i+=10)
    {
        int ch = this->at(i);
        if (CharIsBangla(ch))
        {
            gDocumentBangla = 1;
            if(gBanglaLigMapRev.empty())
            {
                gBanglaLigMapRev = BanglaLigMapReversed();
            }
            return true;
        }
    }
    return false;
}

lString16 processBanglaLigatures(lString16 word)
{
    // bangla lig max length = 7
    int j = (word.length() >= 7 ) ? 7 : word.length();
    for(;  j >= 2; j--)
    {
        for (int c = word.length() - j; c >= 0; c--)
        {
            lUInt32 fastkey = (word.at(c) << 16) + word.at(c + 1);
            if (gBanglaFastLigMap.find(fastkey) == gBanglaFastLigMap.end())
            {
                continue;
            }
            dvngLig lig(word.substr(c, j));
            lChar16 rep = findBanglaLigRev(lig);

            if(isViramaComboBangla(rep))
            {
                if (c == word.length() - j)
                {
                    if (rep == 0xE226 ) rep = 0xE266;
                    if (rep == 0xE225 ) rep = 0xE24C;
                }
                else
                {
                    if (rep == 0xE266 ) rep = 0xE226;
                    if (rep == 0xE24C ) rep = 0xE225;
                }
            }
            //avoiding ra+virama and ya_postform conflict
            if (rep == 0xE272 && c>0 && (word.at(c-1) == 0x09B0 || word.at(c-1) == 0x09F0))
            {
                rep = 0;
            }
            if (rep != 0)
            {
                if(lig.banglaRa && c+j < word.length() && isBangla_consonant(word.at(c+j)) )
                {
                    //LE("rep = 0x%04X, char+1 = 0x%04X",rep,word.at(c+j));
                    continue;
                }
                //LE("found!!, char = 0x%X",rep);
                word.replace(c, j, lString16(&rep, 1));
                c -= j - 2;
            }
        }
    }
    return word;
}

void lString16::StripZWNJ(lString16 *word)
{
    for (int i = 0; i < word->size(); ++i)
    {
        //if (word->at(i) == 0x200C) word->erase(i);
        if (word->at(i) == 0x200C) word->at(i) = 0x200B;
    }
}

void lString16::StripZWNJ_reverse(lString16 *word)
{
    for (int i = 0; i < word->size(); ++i)
    {
        if (word->at(i) == 0x200B) word->at(i) = 0x200C;
    }
}

lString16 lString16::processBanglaText(lString16 str)
{
    if(str.length() < 2)
    {
        return str;
    }
    lString16 res;
    lString16Collection words;

    words.parse(str,' ',true);
    for (int i = 0; i < words.length(); i++)
    {
        lString16 word = words.at(i);

        if(word.length()<2)
        {
            res.append(word);
            res.append(L" ");
            continue;
        }
        word = processBanglaLigatures(word);

        SwitchBanglaReph(&word);
        SwitchBanglaE(&word);
        SwitchBanglaI(&word);
        SwitchBanglaAI(&word);
        SwitchBanglaO(&word);
        SwitchBanglaOU(&word);
        SwitchBanglaRaITa(&word);

        StripZWNJ(&word);

        res.append(word);
        res.append(L" ");
    }
    res.substr(0,res.size()-1);
    return res;
}

//END OF BANGLA ZONE

