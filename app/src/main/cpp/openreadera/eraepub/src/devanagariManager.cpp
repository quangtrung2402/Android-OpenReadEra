//
// Created by Tarasus on 29.06.2020.
//
#include "../include/devanagariManager.h"
#include "../../orebridge/include/ore_log.h"

#include <sys/mman.h>
#include <stdio.h>

#include <vector>
#include <sys/stat.h>

DvngLigMap     gDvngLigMap;
DvngLigMapRev  gDvngLigMapRev;
DvngFastLigMap gDvngFastLigMap;

/*
void parseFontIndexes()
{
    if(gDvngLigMapRev.empty())
    {
        gDvngLigMapRev = DevanagariLigMapReversed();
    }

    int fdin;

    char *src;
    struct stat statbuf;
    size_t fsize = 0;
    fdin = open("sdcard/download/font.txt", O_RDONLY);

    std::vector<std::pair<char*,int>> lines;
    //lines.reserve(1000);

    if ( fstat(fdin, &statbuf) < 0 )
    {
        LE("fstat error");
        return;
    }
    fsize = statbuf.st_size;
    if ((src = (char*)mmap(0, fsize, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED )
    {
        LE("mmap err");
        return;
    }

    char* found = src;
    int pos = 0;
    int lpos = 0;
    while (found!=NULL)
    {
        found = strchr(src+lpos, '\n');
        if(found==NULL)
        {
            std::pair<char*,int> pairB = std::make_pair(src+lpos,fsize-lpos);
            lines.push_back(pairB);
            break;
        }
        pos = found - src;
        std::pair<char*,int> pairA = std::make_pair(src+lpos,pos-lpos);
        lines.push_back(pairA);
        //LE("pos = %d, lpos = %d, len = %d",pos,lpos, pos-lpos);
        lpos = pos + 1;
    }

    for (int i = 0; i < lines.size(); i++)
    {
        int glyphnum = -1;
        std::string line = std::string(lines.at(i).first,lines.at(i).second);
        std::vector<std::string> words = parse(line,' ');
        dvngLig lig;
        lig.len = words.size()-1;

        for (int i = 0; i < words.size(); i++)
        {
            std::string item = words.at(i);
            std::string mod;
            int mod_index = item.find(".");
            if(mod_index != std::string::npos)
            {
                mod = item.substr(mod_index);
                item = item.substr(0,mod_index);
                //LE("mod = %s",mod.c_str());
                //LE("item = %s",item.c_str());
            }
            switch (i)
            {
                case 0: glyphnum = std::atoi(item.c_str()); break;
                case 1: lig.a = std::stoul(item, nullptr, 16); lig.a_mod = mod ; break;
                case 2: lig.b = std::stoul(item, nullptr, 16); lig.b_mod = mod ; break;
                case 3: lig.c = std::stoul(item, nullptr, 16); lig.c_mod = mod ; break;
                case 4: lig.d = std::stoul(item, nullptr, 16); lig.d_mod = mod ; break;
                case 5: lig.e = std::stoul(item, nullptr, 16); lig.e_mod = mod ; break;
                case 6: lig.f = std::stoul(item, nullptr, 16); lig.f_mod = mod ; break;
                default: LE("something's wrong, i = %d",i); break;
            }
        }


        char hashstr[25];
        sprintf(hashstr, "%X%X%X%X%X%X", lig.a, lig.b, lig.c, lig.d, lig.e, lig.f);
        lig.key_hash = lString16(hashstr,strlen(hashstr)).getHash();
        lChar16 ch = findDvngLigRev(lig);
        if(ch == 0)
        {
            //LE("glyphnum = %d 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X ",glyphnum,lig.a,lig.b,lig.c,lig.d,lig.e,lig.f);
            //LE("NotFound");
            continue;
        }

        gDvngLigMap.at(ch).glyphindex = glyphnum;
        gDvngLigMap.at(ch).a_mod = lig.a_mod;
        gDvngLigMap.at(ch).b_mod = lig.b_mod;
        gDvngLigMap.at(ch).c_mod = lig.c_mod;
        gDvngLigMap.at(ch).d_mod = lig.d_mod;
        gDvngLigMap.at(ch).e_mod = lig.e_mod;
        gDvngLigMap.at(ch).f_mod = lig.f_mod;
        //LE("glyphnum = %d a = 0x%X b = 0x%X c = 0x%X d = 0x%X e = 0x%X f = 0x%X ",glyphnum,
           //sgDvngLigMap.at(ch).a,gDvngLigMap.at(ch).b,gDvngLigMap.at(ch).c,gDvngLigMap.at(ch).d,gDvngLigMap.at(ch).e,gDvngLigMap.at(ch).f);
    }
    munmap(src,fsize);

    //debug printout
    //LE("parseFontIndexes debug printout");
    //for (DvngLigMap::iterator it = gDvngLigMap.begin(); it != gDvngLigMap.end() ; it++)
    //{
    //    dvngLig lig = it->second;
    //    LE("Lig 0x%X index = %d (len %d), 0x%X%s 0x%X%s 0x%X%s 0x%X%s 0x%X%s 0x%x%s ",it->first, it->second.glyphindex, it->second.len,
    //       it->second.a,it->second.a_mod.c_str(),
    //       it->second.b,it->second.b_mod.c_str(),
    //       it->second.c,it->second.c_mod.c_str(),
    //       it->second.d,it->second.d_mod.c_str(),
    //       it->second.e,it->second.e_mod.c_str(),
    //       it->second.f,it->second.f_mod.c_str());
    //}
}
*/

std::vector<std::string> parse(const std::string &s, char delimeter)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(delimeter, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}

dvngLig findDvngLig(lChar16 ligature)
{
    if(gDvngLigMap.empty())
    {
        gDvngLigMap = DevanagariLigMap();
    }
    DvngLigMap::iterator it = gDvngLigMap.find(ligature);
    if(it==gDvngLigMap.end())
    {
        return dvngLig();
    }
    return it->second;
}

lChar16 findDvngLigGlyphIndex(lChar16 ligature)
{
    DvngLigMap::iterator it = gDvngLigMap.find(ligature);
    if(it==gDvngLigMap.end())
    {
        //LE("find lig lchar16[0x%X] = unknown",ligature);
        return 0;
    }
    //LE("find lig lchar16[0x%X] = %d",ligature,it->second.glyphindex);
    return it->second.glyphindex;
}

lChar16 findDvngLigRev(dvngLig combo)
{
    if(gDvngLigMapRev.empty())
    {
        gDvngLigMapRev = DevanagariLigMapReversed();
    }
    if(combo.len < 2 || combo.len > 6 )
    {
        return 0;
    }
    DvngLigMapRev::iterator it = gDvngLigMapRev.find(combo);
    if(it==gDvngLigMapRev.end())
    {
        return 0;
    }
    return it->second;
}

std::map <dvngLig, lChar16, Comparator> DevanagariLigMapReversed()
{
    if(!gDvngLigMapRev.empty())
    {
        return gDvngLigMapRev;
    }
    if(gDvngLigMap.empty())
    {
        gDvngLigMap = DevanagariLigMap();
    }
    for (DvngLigMap::iterator i = gDvngLigMap.begin(); i != gDvngLigMap.end(); ++i)
    {
        lUInt32 fastkey = (i->second.a << 16) + i->second.b;
        if(gDvngFastLigMap.find(fastkey) == gDvngFastLigMap.end())
        {
            gDvngFastLigMap[fastkey] = 1;
        }
        gDvngLigMapRev[i->second] = i->first;
    }

    //for (auto i = gDvngFastLigMap.begin(); i != gDvngFastLigMap.end(); ++i)
    //{
    //    LE("fastmap = %d : %d",i->first,i->second);
    //}
    return gDvngLigMapRev;
}

std::map <lChar16, dvngLig> DevanagariLigMap()
{
    if(!gDvngLigMap.empty())
    {
        return gDvngLigMap;
    }
    //native unicode ligatures
    gDvngLigMap.insert(std::make_pair( 0x090c, dvngLig("0x90c 0x93c"))); // letter "I" + Nukta forms letter vocalic "L"
    gDvngLigMap.insert(std::make_pair( 0x0944, dvngLig("0x943 0x93c"))); // vowel sign vocalic "R" + sign Nukta forms vowel sign vocalic "Rr"
    gDvngLigMap.insert(std::make_pair( 0x0950, dvngLig("0x901 0x93c"))); // Candrabindu + sign Nukta forms Om
    gDvngLigMap.insert(std::make_pair( 0x0960, dvngLig("0x90b 0x93c"))); // letter vocalic "R" + sign Nukta forms letter vocalic "Rr"
    gDvngLigMap.insert(std::make_pair( 0x0961, dvngLig("0x908 0x93c"))); // letter "Ii" + sign Nukta forms letter vocalic "LI"
    gDvngLigMap.insert(std::make_pair( 0x0962, dvngLig("0x93f 0x93c"))); // vowel sign "I" + sign Nukta forms vowel sign vocalic "L"
    gDvngLigMap.insert(std::make_pair( 0x0963, dvngLig("0x940 0x93c"))); // vowel sign "Ii" + sign Nukta forms vowel sign vocalic "LI"
    gDvngLigMap.insert(std::make_pair( 0x093d, dvngLig("0x964 0x93c"))); // Danda + sign Nukta forms sign Avagraha

    //DEVANAGARI_START
    //from Noto Sans Devanagari.ttf
    gDvngLigMap.insert(std::make_pair( 0xE001, dvngLig( 136 ,"0x0904 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE002, dvngLig( 137 ,"0x0905 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE003, dvngLig( 138 ,"0x0906 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE004, dvngLig( 139 ,"0x0907 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE005, dvngLig( 140 ,"0x0908 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE006, dvngLig( 141 ,"0x0909 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE007, dvngLig( 142 ,"0x090A 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE008, dvngLig( 143 ,"0x090B 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE009, dvngLig( 144 ,"0x090C 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE00A, dvngLig( 145 ,"0x090D 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE00B, dvngLig( 146 ,"0x090E 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE00C, dvngLig( 147 ,"0x090F 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE00D, dvngLig( 148 ,"0x0910 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE00E, dvngLig( 149 ,"0x0911 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE00F, dvngLig( 150 ,"0x0912 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE010, dvngLig( 151 ,"0x0913 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE011, dvngLig( 152 ,"0x0914 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE012, dvngLig( 153 ,"0x0960 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE013, dvngLig( 154 ,"0x0961 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE014, dvngLig( 155 ,"0x0972 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE015, dvngLig( 156 ,"0x0918 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE016, dvngLig( 157 ,"0x0919 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE017, dvngLig( 158 ,"0x091A 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE018, dvngLig( 159 ,"0x091B 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE019, dvngLig( 160 ,"0x091D 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE01A, dvngLig( 161 ,"0x091E 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE01B, dvngLig( 162 ,"0x091F 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE01C, dvngLig( 163 ,"0x0920 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE01D, dvngLig( 164 ,"0x0923 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE01E, dvngLig( 165 ,"0x0924 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE01F, dvngLig( 166 ,"0x0925 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE020, dvngLig( 167 ,"0x0926 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE021, dvngLig( 168 ,"0x0927 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE022, dvngLig( 169 ,"0x092A 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE023, dvngLig( 170 ,"0x092C 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE024, dvngLig( 171 ,"0x092D 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE025, dvngLig( 172 ,"0x092E 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE026, dvngLig( 173 ,"0x0932 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE027, dvngLig( 174 ,"0x0935 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE028, dvngLig( 175 ,"0x0936 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE029, dvngLig( 176 ,"0x0937 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE02A, dvngLig( 177 ,"0x0938 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE02B, dvngLig( 178 ,"0x0939 0x093C.nukt")));
    gDvngLigMap.insert(std::make_pair( 0xE02C, dvngLig( 179 ,"0x0915 0x094D 0x0937.akhn")));
    gDvngLigMap.insert(std::make_pair( 0xE02D, dvngLig( 180 ,"0x091C 0x094D 0x091E.akhn")));
    gDvngLigMap.insert(std::make_pair( 0xE02E, dvngLig( 181 ,"0x0930 0x094D.rphf")));
    //gDvngLigMap.insert(std::make_pair( 0xE02F, dvngLig( 182 ,"0x0930 0x094D.blwf")));
    gDvngLigMap.insert(std::make_pair( 0xE030, dvngLig( 183 ,"0x0915 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE031, dvngLig( 184 ,"0x0916 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE032, dvngLig( 185 ,"0x0917 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE033, dvngLig( 186 ,"0x0918 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE034, dvngLig( 187 ,"0x0919 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE035, dvngLig( 188 ,"0x091A 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE036, dvngLig( 189 ,"0x091B 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE037, dvngLig( 190 ,"0x091C 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE038, dvngLig( 191 ,"0x091D 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE039, dvngLig( 192 ,"0x091E 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE03A, dvngLig( 193 ,"0x091F 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE03B, dvngLig( 194 ,"0x0920 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE03C, dvngLig( 195 ,"0x0921 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE03D, dvngLig( 196 ,"0x0922 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE03E, dvngLig( 197 ,"0x0923 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE03F, dvngLig( 198 ,"0x0924 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE040, dvngLig( 199 ,"0x0925 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE041, dvngLig( 200 ,"0x0926 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE042, dvngLig( 201 ,"0x0927 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE043, dvngLig( 202 ,"0x0928 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE044, dvngLig( 203 ,"0x092A 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE045, dvngLig( 204 ,"0x092B 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE046, dvngLig( 205 ,"0x092C 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE047, dvngLig( 206 ,"0x092D 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE048, dvngLig( 207 ,"0x092E 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE049, dvngLig( 208 ,"0x092F 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE04A, dvngLig( 209 ,"0x0930 0x094D 0x200D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE04B, dvngLig( 210 ,"0x0932 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE04C, dvngLig( 211 ,"0x0933 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE04D, dvngLig( 212 ,"0x0935 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE04E, dvngLig( 213 ,"0x0936 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE04F, dvngLig( 214 ,"0x0937 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE050, dvngLig( 215 ,"0x0938 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE051, dvngLig( 216 ,"0x0939 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE052, dvngLig( 217 ,"0x0979 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE053, dvngLig( 218 ,"0x097A 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE054, dvngLig( 219 ,"0x0915 0x094D 0x0937.akhn 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE055, dvngLig( 220 ,"0x091C 0x094D 0x091E.akhn 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE056, dvngLig( 221 ,"0x0958 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE057, dvngLig( 222 ,"0x0959 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE058, dvngLig( 223 ,"0x095A 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE059, dvngLig( 224 ,"0x0918 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE05A, dvngLig( 225 ,"0x0919 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE05B, dvngLig( 226 ,"0x091A 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE05C, dvngLig( 227 ,"0x091B 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE05D, dvngLig( 228 ,"0x095B 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE05E, dvngLig( 229 ,"0x091D 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE05F, dvngLig( 230 ,"0x091E 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE060, dvngLig( 231 ,"0x091F 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE061, dvngLig( 232 ,"0x0920 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE062, dvngLig( 233 ,"0x095C 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE063, dvngLig( 234 ,"0x095D 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE064, dvngLig( 235 ,"0x0923 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE065, dvngLig( 236 ,"0x0924 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE066, dvngLig( 237 ,"0x0925 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE067, dvngLig( 238 ,"0x0926 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE068, dvngLig( 239 ,"0x0927 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE069, dvngLig( 240 ,"0x0929 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE06A, dvngLig( 241 ,"0x092A 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE06B, dvngLig( 242 ,"0x095E 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE06C, dvngLig( 243 ,"0x092C 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE06D, dvngLig( 244 ,"0x092D 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE06E, dvngLig( 245 ,"0x092E 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE06F, dvngLig( 246 ,"0x095F 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE070, dvngLig( 247 ,"0x0932 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE071, dvngLig( 248 ,"0x0934 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE072, dvngLig( 249 ,"0x0935 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE073, dvngLig( 250 ,"0x0936 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE074, dvngLig( 251 ,"0x0937 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE075, dvngLig( 252 ,"0x0938 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE076, dvngLig( 253 ,"0x0939 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE077, dvngLig( 254 ,"0x0915 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE078, dvngLig( 255 ,"0x0916 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE079, dvngLig( 256 ,"0x0917 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE07A, dvngLig( 257 ,"0x0918 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE07B, dvngLig( 258 ,"0x0919 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE07C, dvngLig( 259 ,"0x091A 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE07D, dvngLig( 260 ,"0x091B 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE07E, dvngLig( 261 ,"0x091C 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE07F, dvngLig( 262 ,"0x091D 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE080, dvngLig( 263 ,"0x091E 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE081, dvngLig( 264 ,"0x091F 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE082, dvngLig( 265 ,"0x0920 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE083, dvngLig( 266 ,"0x0921 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE084, dvngLig( 267 ,"0x0922 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE085, dvngLig( 268 ,"0x0923 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE086, dvngLig( 269 ,"0x0924 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE087, dvngLig( 270 ,"0x0925 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE088, dvngLig( 271 ,"0x0926 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE089, dvngLig( 272 ,"0x0927 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE08A, dvngLig( 273 ,"0x0928 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE08B, dvngLig( 274 ,"0x092A 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE08C, dvngLig( 275 ,"0x092B 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE08D, dvngLig( 276 ,"0x092C 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE08E, dvngLig( 277 ,"0x092D 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE08F, dvngLig( 278 ,"0x092E 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE090, dvngLig( 279 ,"0x092F 0x094D 0x0930.rkrf")));
    //gDvngLigMap.insert(std::make_pair( 0xE091, dvngLig( 280 ,"0x0930 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE092, dvngLig( 281 ,"0x0932 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE093, dvngLig( 282 ,"0x0933 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE094, dvngLig( 283 ,"0x0935 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE095, dvngLig( 284 ,"0x0936 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE096, dvngLig( 285 ,"0x0937 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE097, dvngLig( 286 ,"0x0938 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE098, dvngLig( 287 ,"0x0939 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE099, dvngLig( 288 ,"0x0978 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE09A, dvngLig( 289 ,"0x0979 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE09B, dvngLig( 290 ,"0x097A 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE09C, dvngLig( 291 ,"0x0915 0x094D 0x0937.akhn 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE09D, dvngLig( 292 ,"0x091C 0x094D 0x091E.akhn 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE09E, dvngLig( 293 ,"0x0958 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE09F, dvngLig( 294 ,"0x0959 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A0, dvngLig( 295 ,"0x095A 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A1, dvngLig( 296 ,"0x0918 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A2, dvngLig( 297 ,"0x0919 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A3, dvngLig( 298 ,"0x091A 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A4, dvngLig( 299 ,"0x091B 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A5, dvngLig( 300 ,"0x095B 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A6, dvngLig( 301 ,"0x091D 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A7, dvngLig( 302 ,"0x091E 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A8, dvngLig( 303 ,"0x091F 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0A9, dvngLig( 304 ,"0x0920 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0AA, dvngLig( 305 ,"0x095C 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0AB, dvngLig( 306 ,"0x095D 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0AC, dvngLig( 307 ,"0x0923 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0AD, dvngLig( 308 ,"0x0924 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0AE, dvngLig( 309 ,"0x0925 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0AF, dvngLig( 310 ,"0x0926 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B0, dvngLig( 311 ,"0x0927 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B1, dvngLig( 312 ,"0x0929 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B2, dvngLig( 313 ,"0x092A 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B3, dvngLig( 314 ,"0x095E 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B4, dvngLig( 315 ,"0x092C 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B5, dvngLig( 316 ,"0x092D 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B6, dvngLig( 317 ,"0x092E 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B7, dvngLig( 318 ,"0x095F 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B8, dvngLig( 319 ,"0x0931 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0B9, dvngLig( 320 ,"0x0932 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0BA, dvngLig( 321 ,"0x0934 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0BB, dvngLig( 322 ,"0x0935 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0BC, dvngLig( 323 ,"0x0936 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0BD, dvngLig( 324 ,"0x0937 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0BE, dvngLig( 325 ,"0x0938 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0BF, dvngLig( 326 ,"0x0939 0x093C.nukt 0x094D 0x0930.rkrf")));
    gDvngLigMap.insert(std::make_pair( 0xE0C0, dvngLig( 327 ,"0x0915 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0C1, dvngLig( 328 ,"0x0916 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0C2, dvngLig( 329 ,"0x0917 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0C3, dvngLig( 330 ,"0x0918 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0C4, dvngLig( 331 ,"0x0919 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0C5, dvngLig( 333 ,"0x091A 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0C6, dvngLig( 334 ,"0x091B 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0C7, dvngLig( 335 ,"0x091C 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0C8, dvngLig( 336 ,"0x091D 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0C9, dvngLig( 337 ,"0x091E 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0CA, dvngLig( 338 ,"0x091F 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0CB, dvngLig( 340 ,"0x0920 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0CC, dvngLig( 342 ,"0x0921 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0CD, dvngLig( 344 ,"0x0922 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0CE, dvngLig( 346 ,"0x0923 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0CF, dvngLig( 347 ,"0x0924 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0D0, dvngLig( 348 ,"0x0925 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0D1, dvngLig( 349 ,"0x0926 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0D2, dvngLig( 350 ,"0x0927 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0D3, dvngLig( 351 ,"0x0928 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0D4, dvngLig( 352 ,"0x092A 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0D5, dvngLig( 353 ,"0x092B 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0D6, dvngLig( 354 ,"0x092C 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0D7, dvngLig( 355 ,"0x092D 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0D8, dvngLig( 356 ,"0x092E 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0D9, dvngLig( 357 ,"0x092F 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0DA, dvngLig( 358 ,"0x0930 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0DB, dvngLig( 359 ,"0x0932 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0DC, dvngLig( 360 ,"0x0933 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0DD, dvngLig( 361 ,"0x0935 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0DE, dvngLig( 362 ,"0x0936 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0DF, dvngLig( 363 ,"0x0937 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0E0, dvngLig( 364 ,"0x0938 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0E1, dvngLig( 365 ,"0x0939 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0E2, dvngLig( 366 ,"0x0979 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0E3, dvngLig( 367 ,"0x097A 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0E4, dvngLig( 368 ,"0x0915 0x094D 0x0937.akhn 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0E5, dvngLig( 369 ,"0x091C 0x094D 0x091E.akhn 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0E6, dvngLig( 370 ,"0x0958 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0E7, dvngLig( 371 ,"0x0959 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0E8, dvngLig( 372 ,"0x095A 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0E9, dvngLig( 373 ,"0x0918 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0EA, dvngLig( 374 ,"0x0919 0x093C.nukt 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0EB, dvngLig( 375 ,"0x091A 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0EC, dvngLig( 376 ,"0x091B 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0ED, dvngLig( 377 ,"0x095B 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0EE, dvngLig( 378 ,"0x091D 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0EF, dvngLig( 379 ,"0x091E 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0F0, dvngLig( 380 ,"0x091F 0x093C.nukt 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0F1, dvngLig( 381 ,"0x0920 0x093C.nukt 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0F2, dvngLig( 382 ,"0x095C 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0F3, dvngLig( 383 ,"0x095D 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0F4, dvngLig( 384 ,"0x0923 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0F5, dvngLig( 385 ,"0x0924 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0F6, dvngLig( 386 ,"0x0925 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0F7, dvngLig( 387 ,"0x0926 0x093C.nukt 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE0F8, dvngLig( 388 ,"0x0927 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0F9, dvngLig( 389 ,"0x0929 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0FA, dvngLig( 390 ,"0x092A 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0FB, dvngLig( 391 ,"0x095E 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0FC, dvngLig( 392 ,"0x092C 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0FD, dvngLig( 393 ,"0x092D 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0FE, dvngLig( 394 ,"0x092E 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE0FF, dvngLig( 395 ,"0x095F 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE100, dvngLig( 396 ,"0x0932 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE101, dvngLig( 397 ,"0x0934 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE102, dvngLig( 398 ,"0x0935 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE103, dvngLig( 399 ,"0x0936 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE104, dvngLig( 400 ,"0x0937 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE105, dvngLig( 401 ,"0x0938 0x093C.nukt 0x094D 0x0930.rkrf 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE106, dvngLig( 402 ,"0x0939 0x093C.nukt 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE107, dvngLig( 403 ,"0x0939 0x0941.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE108, dvngLig( 404 ,"0x0939 0x0942.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE109, dvngLig( 405 ,"0x0939 0x0943.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE10A, dvngLig( 406 ,"0x0939 0x0944.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE10B, dvngLig( 407 ,"0x0939 0x093C.nukt 0x0941.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE10C, dvngLig( 408 ,"0x0939 0x093C.nukt 0x0942.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE10D, dvngLig( 409 ,"0x0939 0x093C.nukt 0x0943.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE10E, dvngLig( 410 ,"0x0939 0x093C.nukt 0x0944.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE10F, dvngLig( 411 ,"0x0939 0x094D 0x0930.rkrf 0x0941.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE110, dvngLig( 412 ,"0x0939 0x094D 0x0930.rkrf 0x0942.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE111, dvngLig( 413 ,"0x0930 0x0941.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE112, dvngLig( 414 ,"0x0930 0x0942.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE113, dvngLig( 415 ,"0x0926 0x0941.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE114, dvngLig( 416 ,"0x0926 0x0942.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE115, dvngLig( 417 ,"0x0926 0x0943.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE116, dvngLig( 418 ,"0x0931 0x0941.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE117, dvngLig( 419 ,"0x0931 0x0942.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE118, dvngLig( 420 ,"0x0926 0x093C 0x0941.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE119, dvngLig( 421 ,"0x0926 0x093C 0x0942.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE11A, dvngLig( 422 ,"0x0926 0x093C 0x0943.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE11B, dvngLig( 423 ,"0x093A 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE11C, dvngLig( 424 ,"0x093A 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE11D, dvngLig( 425 ,"0x093A 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE11E, dvngLig( 426 ,"0x093B 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE11F, dvngLig( 427 ,"0x093B 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE120, dvngLig( 428 ,"0x093B 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE121, dvngLig( 429 ,"0x0940 0x0902.abvs")));
    //gDvngLigMap.insert(std::make_pair( 0xE122, dvngLig( 430 ,"0x0940 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE123, dvngLig( 431 ,"0x0940 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE124, dvngLig( 432 ,"0x0945 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE125, dvngLig( 433 ,"0x0945 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE126, dvngLig( 434 ,"0x0945 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE127, dvngLig( 435 ,"0x0946 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE128, dvngLig( 436 ,"0x0946 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE129, dvngLig( 437 ,"0x0946 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE12A, dvngLig( 438 ,"0x0947 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE12B, dvngLig( 439 ,"0x0947 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE12C, dvngLig( 440 ,"0x0947 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE12D, dvngLig( 441 ,"0x0948 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE12E, dvngLig( 442 ,"0x0948 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE12F, dvngLig( 443 ,"0x0948 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE130, dvngLig( 444 ,"0x0949 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE131, dvngLig( 445 ,"0x0949 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE132, dvngLig( 446 ,"0x0949 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE133, dvngLig( 447 ,"0x094A 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE134, dvngLig( 448 ,"0x094A 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE135, dvngLig( 449 ,"0x094A 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE136, dvngLig( 450 ,"0x094B 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE137, dvngLig( 451 ,"0x094B 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE138, dvngLig( 452 ,"0x094B 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE139, dvngLig( 453 ,"0x094C 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE13A, dvngLig( 454 ,"0x094C 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE13B, dvngLig( 455 ,"0x094C 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE13C, dvngLig( 456 ,"0x094F 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE13D, dvngLig( 457 ,"0x094F 0x0930 0x094D.rphf.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE13E, dvngLig( 458 ,"0x094F 0x0930 0x094D.rphf 0x0902.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE13F, dvngLig( 459 ,"0x0930 0x094D.rphf 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE140, dvngLig( 460 ,"0x0904 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE141, dvngLig( 461 ,"0x0908 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE142, dvngLig( 462 ,"0x090D 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE143, dvngLig( 463 ,"0x090E 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE144, dvngLig( 464 ,"0x0910 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE145, dvngLig( 465 ,"0x0911 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE146, dvngLig( 466 ,"0x0912 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE147, dvngLig( 467 ,"0x0913 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE148, dvngLig( 468 ,"0x0914 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE149, dvngLig( 469 ,"0x0972 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE14A, dvngLig( 470 ,"0x0973 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE14B, dvngLig( 471 ,"0x0974 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE14C, dvngLig( 472 ,"0x0975 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE14D, dvngLig( 473 ,"0x0904 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE14E, dvngLig( 474 ,"0x0908 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE14F, dvngLig( 475 ,"0x090D 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE150, dvngLig( 476 ,"0x090E 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE151, dvngLig( 477 ,"0x0910 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE152, dvngLig( 478 ,"0x0911 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE153, dvngLig( 479 ,"0x0912 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE154, dvngLig( 480 ,"0x0913 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE155, dvngLig( 481 ,"0x0914 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE156, dvngLig( 482 ,"0x0972 0x093C.nukt 0x0902.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE157, dvngLig( 483 ,"0x0915 0x094D.half 0x0924.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE158, dvngLig( 484 ,"0x0919 0x094D 0x0917.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE159, dvngLig( 485 ,"0x0919 0x094D 0x092E.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE15A, dvngLig( 486 ,"0x0919 0x094D 0x092F.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE15B, dvngLig( 487 ,"0x091B 0x094D.half 0x092F.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE15C, dvngLig( 488 ,"0x091E 0x094D.half 0x091C.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE15D, dvngLig( 489 ,"0x091F 0x094D 0x091F.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE15E, dvngLig( 490 ,"0x091F 0x094D 0x091F 0x0942.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE15F, dvngLig( 491 ,"0x091F 0x094D 0x0920.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE160, dvngLig( 492 ,"0x091F 0x094D 0x0920 0x0942.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE161, dvngLig( 493 ,"0x091F 0x094D 0x092F.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE162, dvngLig( 494 ,"0x0920 0x094D 0x0920.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE163, dvngLig( 495 ,"0x0920 0x094D 0x092F.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE164, dvngLig( 496 ,"0x0921 0x094D 0x0922.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE165, dvngLig( 497 ,"0x0921 0x094D 0x0921.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE166, dvngLig( 498 ,"0x0921 0x094D 0x0921 0x0942.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE167, dvngLig( 499 ,"0x0921 0x094D 0x092F.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE168, dvngLig( 500 ,"0x0922 0x094D 0x0922.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE169, dvngLig( 501 ,"0x0922 0x094D 0x092F.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE16A, dvngLig( 502 ,"0x0924 0x094D.half 0x0924.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE16B, dvngLig( 503 ,"0x0924 0x094D.half 0x0924 0x094D.half.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE16C, dvngLig( 504 ,"0x0926 0x094D 0x0918.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE16D, dvngLig( 505 ,"0x0926 0x094D 0x0917.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE16E, dvngLig( 506 ,"0x0926 0x094D 0x092C.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE16F, dvngLig( 507 ,"0x0926 0x094D 0x092D.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE170, dvngLig( 508 ,"0x0926 0x094D 0x0935.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE171, dvngLig( 509 ,"0x0926 0x094D 0x0927.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE172, dvngLig( 510 ,"0x0926 0x094D 0x0927 0x094D.half 0x092F.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE173, dvngLig( 511 ,"0x0926 0x094D 0x0926.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE174, dvngLig( 512 ,"0x0926 0x094D 0x092E.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE175, dvngLig( 513 ,"0x0926 0x094D 0x092F.cjct")));
    gDvngLigMap.insert(std::make_pair( 0xE176, dvngLig( 514 ,"0x0926 0x094D 0x092F 0x094D.half.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE177, dvngLig( 515 ,"0x0928 0x094D.half 0x0928.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE178, dvngLig( 516 ,"0x092A 0x094D.half 0x0928.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE179, dvngLig( 517 ,"0x0935 0x094D.half 0x092F.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE17A, dvngLig( 518 ,"0x0936 0x094D.half 0x091A.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE17B, dvngLig( 519 ,"0x0936 0x094D.half 0x091A 0x094D.half.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE17C, dvngLig( 520 ,"0x0936 0x094D.half 0x0935.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE17D, dvngLig( 521 ,"0x0936 0x094D.half 0x0935 0x094D.half.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE17E, dvngLig( 522 ,"0x0936 0x094D.half 0x0932.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE17F, dvngLig( 523 ,"0x0936 0x094D.half 0x0928.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE180, dvngLig( 524 ,"0x0937 0x094D.half 0x091F.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE181, dvngLig( 525 ,"0x0937 0x094D.half 0x091F 0x094D 0x0930.rkrf.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE182, dvngLig( 526 ,"0x0937 0x094D.half 0x0920.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE183, dvngLig( 527 ,"0x0937 0x094D.half 0x0920 0x094D 0x0930.rkrf.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE184, dvngLig( 528 ,"0x0939 0x094D.half 0x0923.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE185, dvngLig( 529 ,"0x0939 0x094D.half 0x0928.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE186, dvngLig( 530 ,"0x0939 0x094D.half 0x092E.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE187, dvngLig( 531 ,"0x0939 0x094D.half 0x092F.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE188, dvngLig( 532 ,"0x0939 0x094D.half 0x0932.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE189, dvngLig( 533 ,"0x0939 0x094D.half 0x0935.pres")));
    gDvngLigMap.insert(std::make_pair( 0xE18A, dvngLig( 534 ,"0x091B 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE18B, dvngLig( 535 ,"0x091B 0x093C.nukt 0x094D.half")));
    gDvngLigMap.insert(std::make_pair( 0xE18C, dvngLig( 536 ,"0x091B 0x094D.half 0x0930 0x094D.blwf.vatu")));
    gDvngLigMap.insert(std::make_pair( 0xE18D, dvngLig( 537 ,"0x091B 0x093C.nukt 0x094D.half 0x0930 0x094D.blwf.vatu")));
    //gDvngLigMap.insert(std::make_pair( 0xE18E, dvngLig( 599 ,"0x093C 0x0941.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE18F, dvngLig( 602 ,"0x093C 0x0942.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE190, dvngLig( 604 ,"0x093C 0x0943.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE191, dvngLig( 606 ,"0x093C 0x0944.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE192, dvngLig( 609 ,"0x093C 0x0962.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE193, dvngLig( 611 ,"0x093C 0x0963.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE194, dvngLig( 614 ,"0x093C 0x094D.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE195, dvngLig( 615 ,"0x093C 0x0956.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE196, dvngLig( 617 ,"0x093C 0x0957.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE197, dvngLig( 636 ,"0x0930 0x094D.blwf 0x0941.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE198, dvngLig( 639 ,"0x0930 0x094D.blwf 0x0942.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE199, dvngLig( 642 ,"0x0930 0x094D.blwf 0x0943.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE19A, dvngLig( 645 ,"0x0930 0x094D.blwf 0x0944.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE19B, dvngLig( 646 ,"0x0930 0x094D.blwf 0x0962.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE19C, dvngLig( 647 ,"0x0930 0x094D.blwf 0x0963.blws")));
    //gDvngLigMap.insert(std::make_pair( 0xE19D, dvngLig( 648 ,"0x0930 0x094D.blwf 0x094D.blws")));
    gDvngLigMap.insert(std::make_pair( 0xE19E, dvngLig( 667 ,"0x0930 0x094D.rphf 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE19F, dvngLig( 668 ,"0x093A 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A0, dvngLig( 669 ,"0x093B 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A1, dvngLig( 670 ,"0x0945 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A2, dvngLig( 671 ,"0x0946 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A3, dvngLig( 672 ,"0x0947 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A4, dvngLig( 673 ,"0x0948 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A5, dvngLig( 674 ,"0x0949 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A6, dvngLig( 675 ,"0x094A 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A7, dvngLig( 676 ,"0x094B 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A8, dvngLig( 677 ,"0x094C 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1A9, dvngLig( 678 ,"0x094F 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1AA, dvngLig( 679 ,"0x0940 0x0901.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1AB, dvngLig( 683 ,"0x093A 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1AC, dvngLig( 684 ,"0x093B 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1AD, dvngLig( 685 ,"0x0945 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1AE, dvngLig( 686 ,"0x0946 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1AF, dvngLig( 687 ,"0x0947 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B0, dvngLig( 688 ,"0x0948 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B1, dvngLig( 689 ,"0x0949 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B2, dvngLig( 690 ,"0x094A 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B3, dvngLig( 691 ,"0x094B 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B4, dvngLig( 692 ,"0x094C 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B5, dvngLig( 693 ,"0x094F 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B6, dvngLig( 706 ,"0x0940 0x0930 0x094D.rphf 0x0901.abvs.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B7, dvngLig( 843 ,"0xA8E1 0xA8E1.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B8, dvngLig( 844 ,"0xA8E2 0xA8EB.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1B9, dvngLig( 845 ,"0xA8E3 0xA8EC.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1BA, dvngLig( 846 ,"0xA8E1 0xA8EF.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1BB, dvngLig( 847 ,"0xA8E2 0xA8EF.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1BC, dvngLig( 848 ,"0xA8E3 0xA8EF.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1BD, dvngLig( 849 ,"0xA8E4 0xA8EF.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1BE, dvngLig( 850 ,"0xA8E5 0xA8EF.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1BF, dvngLig( 851 ,"0xA8E2 0xA8F1.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1C0, dvngLig( 852 ,"0xA8E2 0x1CD0.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1C1, dvngLig( 853 ,"0xA8F0 0xA8EF.abvs")));
    gDvngLigMap.insert(std::make_pair( 0xE1C2, dvngLig( 854 ,"0x0903 0x1CE2.psts")));
    gDvngLigMap.insert(std::make_pair( 0xE1C3, dvngLig( 855 ,"0x0903 0x1CE4.psts")));
    gDvngLigMap.insert(std::make_pair( 0xE1C4, dvngLig( 856 ,"0x0903 0x1CE5.psts")));
    gDvngLigMap.insert(std::make_pair( 0xE1C5, dvngLig( 857 ,"0x0903 0x1CE8.psts")));
    //DEVANAGARI_END
    //gDvngLigMap.insert(std::make_pair( 0xE1C6, dvngLig( 584 ,""))); // Log devanagari vowel "i" jumping over two letters


    //debug printout
    //LE("gDvngLigMap debug printout");
    //for (DvngLigMap::iterator it = gDvngLigMap.begin(); it != gDvngLigMap.end() ; it++)
    //{
    //    dvngLig lig = it->second;
    //    LE("Lig 0x%X index = %d (len %d), 0x%X%s 0x%X%s 0x%X%s 0x%X%s 0x%X%s 0x%x%s ",it->first, it->second.glyphindex, it->second.len,
    //       it->second.a,it->second.a_mod.c_str(),
    //       it->second.b,it->second.b_mod.c_str(),
    //       it->second.c,it->second.c_mod.c_str(),
    //       it->second.d,it->second.d_mod.c_str(),
    //       it->second.e,it->second.e_mod.c_str(),
    //       it->second.f,it->second.f_mod.c_str());
    //}

    return gDvngLigMap;
}

lString16 restoreDvngWord(lString16 in)
{
    if(DVNG_DISPLAY_ENABLE == 0 || gDocumentDvng == 0)
    {
        return in;
    }
    lString16::SwitchDvngReph_reverse(&in);
    lString16::SwitchDvngI_reverse(&in);
    for (int i = 0; i < in.length(); i++)
    {
        if (in[i] < DEVANAGARI_START || in[i] > DEVANAGARI_END)
        {
            continue;
        }

        dvngLig lig = findDvngLig(in[i]);
        if (lig.isNull() || lig.len == 0 || lig.len > 6)
        {
            continue;
        }

        lString16 rep;
        switch (lig.len)
        {
            case 0: break;
            case 1: rep += lig.a; break;
            case 2: rep += lig.a; rep += lig.b; break;
            case 3: rep += lig.a; rep += lig.b; rep += lig.c; break;
            case 4: rep += lig.a; rep += lig.b; rep += lig.c; rep += lig.d; break;
            case 5: rep += lig.a; rep += lig.b; rep += lig.c; rep += lig.d; rep += lig.e; break;
            case 6: rep += lig.a; rep += lig.b; rep += lig.c; rep += lig.d; rep += lig.e; rep += lig.f; break;
            case 7: rep += lig.a; rep += lig.b; rep += lig.c; rep += lig.d; rep += lig.e; rep += lig.f; rep += lig.g;break;
            case 8: rep += lig.a; rep += lig.b; rep += lig.c; rep += lig.d; rep += lig.e; rep += lig.f; rep += lig.g; rep += lig.h;break;
            case 9: rep += lig.a; rep += lig.b; rep += lig.c; rep += lig.d; rep += lig.e; rep += lig.f; rep += lig.g; rep += lig.h; rep += lig.i;break;
           case 10: rep += lig.a; rep += lig.b; rep += lig.c; rep += lig.d; rep += lig.e; rep += lig.f; rep += lig.g; rep += lig.h; rep += lig.i; rep += lig.j; break;
        }
        lString16 firsthalf = in.substr(0,i);
        lString16 lasthalf  = in.substr(i+1,in.length()-i);
        in = firsthalf + rep + lasthalf;
    }
    return in;
}