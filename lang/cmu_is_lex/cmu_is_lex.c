/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2013                            */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*  is Lexical function                                            */
/*                                                                       */
/*************************************************************************/
#include "flite.h"
#include "cst_val.h"
#include "cst_voice.h"
#include "cst_lexicon.h"
#include "cst_ffeatures.h"
#include "cmu_is_lex.h"

extern const int cmu_is_lex_entry[];
extern const unsigned char cmu_is_lex_data[];
extern const int cmu_is_lex_num_entries;
extern const int cmu_is_lex_num_bytes;
extern const char * const cmu_is_lex_phone_table[71];
extern const char * const cmu_is_lex_phones_huff_table[];
extern const char * const cmu_is_lex_entries_huff_table[];

static int cmu_is_is_vowel(const char *p);
static int cmu_is_is_silence(const char *p);
static int cmu_is_has_vowel_in_list(const cst_val *v);
static int cmu_is_has_vowel_in_syl(const cst_item *i);
static int cmu_is_sonority(const char *p);

// cst_val *cmu_is_lex_lts_function(const struct lexicon_struct *l, 
//                                    const char *word, const char *pos)
// {
//     return NULL;
// }

static int cmu_is_is_silence(const char *p)
{
    if (cst_streq(p,"pau"))
	return TRUE;
    else
	return FALSE;
}

static int cmu_is_has_vowel_in_list(const cst_val *v)
{
    const cst_val *t;

    for (t=v; t; t=val_cdr(t))
	if (cmu_is_is_vowel(val_string(val_car(t))))
	    return TRUE;
    return FALSE;
}

static int cmu_is_has_vowel_in_syl(const cst_item *i)
{
    const cst_item *n;

    for (n=i; n; n=item_prev(n))
	if (cmu_is_is_vowel(item_feat_string(n,"name")))
	    return TRUE;
    return FALSE;
}

static int cmu_is_is_vowel(const char *p)
{
    /* this happens to work for icelandic SAMPA phoneset */
    if (strchr("aeiouOEYI9",p[0]) == NULL)
	return FALSE;
    else
	return TRUE;
}

static int cmu_is_sonority(const char *p)
{
    /* A bunch of hacks for US English phoneset */
    if (cmu_is_is_vowel(p) || (cmu_is_is_silence(p)))
	return 5;
    else if ((sizeof(p) < 2 || strchr("0",p[2]) == NULL) && strchr("wylr",p[0]) != NULL)
	return 4;  /* glides/liquids */
    else if (strchr("nmNJ",p[0]) != NULL)
	return 3;  /* nasals */
    else if (strchr("bdgjvzGD",p[0]) != NULL)
	return 2;  /* voiced obstruents */
    else
	return 1;
}

int cmu_is_syl_boundary(const cst_item *i,const cst_val *rest)
{
    // TODO adapt to is
    /* Returns TRUE if this should be a syllable boundary */
    /* This is of course phone set dependent              */
    int p, n, nn;

    if (rest == NULL)
	return TRUE;
    else if (cmu_is_is_silence(val_string(val_car(rest))))
	return TRUE;
    else if (!cmu_is_has_vowel_in_list(rest)) /* no more vowels so rest *all* coda */
	return FALSE;
    else if (!cmu_is_has_vowel_in_syl(i))  /* need a vowel */
	return FALSE;
    else if (cmu_is_is_vowel(val_string(val_car(rest))))
	return TRUE;
    else if (val_cdr(rest) == NULL)
	return FALSE;
    else 
    {   /* so there is following vowel, and multiple phones left */
	p = cmu_is_sonority(item_feat_string(i,"name"));
	n = cmu_is_sonority(val_string(val_car(rest)));
	nn = cmu_is_sonority(val_string(val_car(val_cdr(rest))));

	if ((p <= n) && (n <= nn))
	    return TRUE;
	else
	    return FALSE;
    }
}

static int cmu_is_lex_dist_to_vowel(const cst_val *rest)
{
    // TODO adapt to is
    if (rest == 0)
        return 0;  /* shouldn't get here */
    else if (cmu_is_is_vowel(val_string(val_car(rest))))
        return 0;
    else
        return 1+cmu_is_lex_dist_to_vowel(val_cdr(rest));
}

static const char * const cmu_is_lex_onset_trigrams[] = {
    // TODO adapt to is
    "str", "spy", "spr", "spl", "sky", "skw", "skr", "skl", NULL
};
static const char * const cmu_is_lex_onset_bigrams[] = {
    // TODO adapt to is
    "sv", "sj", "sr",
    "pv", "pj", "pr", 
    "tv", "tj", "tr",
    "kv", "kj", "kr",
    "fr",


    "zw", "zl",
    "vy", "vr", "vl",
    "thw", "thr",
    "ty", "tw",
    "tr", /* "ts", */
    "shw", "shr", "shn", "shm", "shl",
    "sw", "sv", "st", "sr", "sp", "sn", "sm", "sl", "sk", "sf",
    "py", "pw", "pr", "pl",
    "ny",
    "my", "mr",
    "ly",
    "ky", "kw", "kr", "kl",
    "hhy", "hhw", "hhr", "hhl",
    "gy", "gw", "gr", "gl", 
    "fy", "fr", "fl", 
    "dy", "dw", "dr",
    "by", "bw", "br", "bl",
    NULL
};

static int cmu_is_lex_onset_bigram(const cst_val *rest)
{
    // TODO adapt to is
    char x[10];
    int i;

    cst_sprintf(x,"%s%s",val_string(val_car(rest)),
           val_string(val_car(val_cdr(rest))));
    for (i=0; cmu_is_lex_onset_bigrams[i]; i++)
        if (cst_streq(x,cmu_is_lex_onset_bigrams[i]))
            return TRUE;
    return FALSE;
}

static int cmu_is_lex_onset_trigram(const cst_val *rest)
{
    // TODO adapt to is
    char x[15];
    int i;

    cst_sprintf(x,"%s%s%s",val_string(val_car(rest)),
           val_string(val_car(val_cdr(rest))),
           val_string(val_car(val_cdr(val_cdr(rest)))));
    for (i=0; cmu_is_lex_onset_trigrams[i]; i++)
        if (cst_streq(x,cmu_is_lex_onset_trigrams[i]))
            return TRUE;
    return FALSE;
}

int cmu_is_syl_boundary_mo(const cst_item *i,const cst_val *rest)
{
    // TODO adapt to is
    /* syl boundary maximal onset */
    int d2v;

    if (rest == NULL)
	return TRUE;
    else if (cmu_is_is_silence(val_string(val_car(rest))))
	return TRUE;
    else if (!cmu_is_has_vowel_in_list(rest)) 
        /* no more vowels so rest *all* coda */
	return FALSE;
    else if (!cmu_is_has_vowel_in_syl(i))  /* need a vowel */
        /* no vowel yet in syl so keep copying */
	return FALSE;
    else if (cmu_is_is_vowel(val_string(val_car(rest))))
        /* next is a vowel, syl has vowel, so this is a break */
	return TRUE;
    else if (cst_streq("ng",val_string(val_car(rest))))
        /* next is "ng" which can't start a word internal syl */
	return FALSE;
    else 
    {
        /* want to know if from rest to the next vowel is a valid onset */
        d2v = cmu_is_lex_dist_to_vowel(rest);
        if (d2v < 2)
            return TRUE;
        else if (d2v > 3)
            return FALSE;
        else if (d2v == 2) 
            return cmu_is_lex_onset_bigram(rest);
        else /* if (d2v == 3) */
            return cmu_is_lex_onset_trigram(rest);
        return TRUE;
    }

}

cst_utterance *cmu_is_lex_postlex(cst_utterance *u)
{
    return u;
}

cst_lexicon cmu_is_lex;
cst_lts_rules cmu_is_lts_rules;
extern const char * const cmu_is_lts_phone_table[];
extern const char * const cmu_is_lts_letter_table[];
extern const cst_lts_addr cmu_is_lts_letter_index[];
extern const cst_lts_model cmu_is_lts_model[];

cst_lexicon *cmu_is_lex_init(void)
{
    /* Should it be global const or dynamic */
    /* Can make lts_rules just a cart tree like others */

    if (cmu_is_lts_rules.name)
    return &cmu_is_lex;  /* Already initialized */

    cmu_is_lts_rules.name = "cmu_is";
    cmu_is_lts_rules.letter_index = cmu_is_lts_letter_index;

    cmu_is_lts_rules.models = cmu_is_lts_model;

    cmu_is_lts_rules.phone_table = cmu_is_lts_phone_table;
    cmu_is_lts_rules.context_window_size = 4;
    cmu_is_lts_rules.context_extra_feats = 1;
    cmu_is_lts_rules.letter_table = 0 /* cmu_is_lts_letter_table */;

    cmu_is_lex.name = "cmu_is";
    cmu_is_lex.num_entries = cmu_is_lex_num_entries;

    cmu_is_lex.data = (unsigned char *)(void *)cmu_is_lex_data;

    cmu_is_lex.num_bytes = cmu_is_lex_num_bytes;
    cmu_is_lex.phone_table = (char **) cmu_is_lex_phone_table;
    cmu_is_lex.syl_boundary = cmu_is_syl_boundary_mo;
    cmu_is_lex.lts_rule_set = (cst_lts_rules *) &cmu_is_lts_rules;

    cmu_is_lex.phone_hufftable = cmu_is_lex_phones_huff_table;
    cmu_is_lex.entry_hufftable = cmu_is_lex_entries_huff_table;

    cmu_is_lex.postlex = cmu_is_lex_postlex;

    return &cmu_is_lex;

    // cst_lexicon *l;

    // l = cst_alloc(cst_lexicon,1);
    // l->name = "cmu_is_lex";

    // l->lts_function = cmu_is_lex_lts_function;

    // return l;

}
