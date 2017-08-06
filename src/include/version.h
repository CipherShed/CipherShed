#ifndef _cs_include_version_h_
#define _cs_include_version_h_

//Marketing version
#define _CS_VERSION_1Market 0
// Major version
#define _CS_VERSION_2Major 7
//update this for new branches/releases
#define _CS_VERSION_3Minor 5

//This is a candidate for auto generation
#define _CS_VERSION_4Build 0

// This indicates if we are using the 5th digit of the version
#undef _CS_VERSION_HAS_5th
#ifdef _CS_VERSION_HAS_5th
#define _CS_VERSION_5Patch 0
#else
#undef  _CS_VERSION_5Patch
#endif



#define _cs_include_version_dot_ .
#define _cs_include_version_f_str_(s) _cs_include_version_f_str2_(s)
#define _cs_include_version_f_str2_(s) #s
#define _cs_include_version_f_2merge2_(a,b) a##b
#define _cs_include_version_f_2merge_(a,b) _cs_include_version_f_2merge2_(a,b)
#define _cs_include_version_f_2mergedotted2_(a,b) a##.##b
#define _cs_include_version_f_2mergedotted_(a,b) _cs_include_version_f_2mergedotted2_(a,b)
#define _cs_include_version_f_3mergedotted2_(a,b,c) a##.##b##.##c
#define _cs_include_version_f_3mergedotted_(a,b,c) _cs_include_version_f_3mergedotted2_(a,b,c)
#define _cs_include_version_f_4mergedotted2_(a,b,c,d) a##.##b##.##c##.##d
#define _cs_include_version_f_4mergedotted_(a,b,c,d) _cs_include_version_f_4mergedotted2_(a,b,c,d)
#define _cs_include_version_f_5mergedotted2_(a,b,c,d,e) a##.##b##.##c##.##d##.##e
#define _cs_include_version_f_5mergedotted_(a,b,c,d,e) _cs_include_version_f_5mergedotted2_(a,b,c,d,e)


#ifdef _CS_VERSION_HAS_5th
#define _CS_VERSION_DOTED_SUFFIX5 _cs_include_version_f_2merge_(_cs_include_version_dot_,_CS_VERSION_5Patch)
#define str_CS_VERSION_5Patch _cs_include_version_f_str_(_CS_VERSION_5Patch)
#define _CS_VERSION_DOTTED_5RAW _cs_include_version_f_5mergedotted_(_CS_VERSION_1Market,_CS_VERSION_2Major,_CS_VERSION_3Minor,_CS_VERSION_4Build,_CS_VERSION_5Patch)
#else
#define _CS_VERSION_DOTED_SUFFIX5
#define str_CS_VERSION_5Patch ""
#define _CS_VERSION_DOTTED_5RAW _cs_include_version_f_4mergedotted_(_CS_VERSION_1Market,_CS_VERSION_2Major,_CS_VERSION_3Minor,_CS_VERSION_4Build)
#endif

#define _CS_VERSION_DOTTED_2RAW _cs_include_version_f_2mergedotted_(_CS_VERSION_1Market,_CS_VERSION_2Major)
#define _CS_VERSION_DOTTED_3RAW _cs_include_version_f_3mergedotted_(_CS_VERSION_1Market,_CS_VERSION_2Major,_CS_VERSION_3Minor)
#define _CS_VERSION_DOTTED_4RAW _cs_include_version_f_4mergedotted_(_CS_VERSION_1Market,_CS_VERSION_2Major,_CS_VERSION_3Minor,_CS_VERSION_4Build)

#define _CS_VERSION_DOTTED_2STR _cs_include_version_f_str_(_CS_VERSION_DOTTED_2RAW)
#define _CS_VERSION_DOTTED_3STR _cs_include_version_f_str_(_CS_VERSION_DOTTED_3RAW)
#define _CS_VERSION_DOTTED_4STR _cs_include_version_f_str_(_CS_VERSION_DOTTED_4RAW)
#define _CS_VERSION_DOTTED_5STR _cs_include_version_f_str_(_CS_VERSION_DOTTED_5RAW)


#define _CS_VERSION_STR_1Market _cs_include_version_f_str_(_CS_VERSION_1Market)
#define _CS_VERSION_STR_2Major _cs_include_version_f_str_(_CS_VERSION_2Major)
#define _CS_VERSION_STR_3Minor _cs_include_version_f_str_(_CS_VERSION_3Minor)
#define _CS_VERSION_STR_4Build _cs_include_version_f_str_(_CS_VERSION_4Build)
#define _CS_VERSION_STR_SUFFIX_5Patch _cs_include_version_f_str_(_CS_VERSION_DOTED_SUFFIX5)

#define _CS_VERSION_2MSB _CS_VERSION_STR_1Market "." _CS_VERSION_STR_2Major
#define _CS_VERSION_3MSB _CS_VERSION_STR_1Market "." _CS_VERSION_STR_2Major "." _CS_VERSION_STR_3Minor
#define _CS_VERSION_4MSB _CS_VERSION_STR_1Market "." _CS_VERSION_STR_2Major "." _CS_VERSION_STR_3Minor "." _CS_VERSION_STR_4Build
#define _CS_VERSION_5MSB _CS_VERSION_STR_1Market "." _CS_VERSION_STR_2Major "." _CS_VERSION_STR_3Minor "." _CS_VERSION_STR_4Build _CS_VERSION_STR_SUFFIX_5Patch

#endif
