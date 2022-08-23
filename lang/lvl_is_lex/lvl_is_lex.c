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
#include "lvl_is_lex.h"



extern const int lvl_is_lex_entry[];
extern const unsigned char lvl_is_lex_data[];
extern const int lvl_is_lex_num_entries;
extern const int lvl_is_lex_num_bytes;
extern const char * const lvl_is_lex_phone_table[];
extern const char * const lvl_is_lex_phones_huff_table[];
extern const char * const lvl_is_lex_entries_huff_table[];


cst_lexicon lvl_is_lex;
cst_lts_rules lvl_is_lts_rules;
extern const char * const lvl_is_lts_phone_table[];
extern const char * const lvl_is_lts_letter_table[];
extern const cst_lts_addr lvl_is_lts_letter_index[];
extern const cst_lts_model lvl_is_lts_model[];

static int lvl_is_is_vowel(const char *p);
static int lvl_is_is_silence(const char *p);
static int lvl_is_has_vowel_in_list(const cst_val *v);
static int lvl_is_has_vowel_in_syl(const cst_item *i);
static int lvl_is_sonority(const char *p);

// cst_val *lvl_is_lex_lts_function(const struct lexicon_struct *l, 
//                                    const char *word, const char *pos)
// {
//     return NULL;
// }

static int lvl_is_is_silence(const char *p)
{
    if (cst_streq(p,"pau"))
	return TRUE;
    else
	return FALSE;
}

static int lvl_is_has_vowel_in_list(const cst_val *v)
{
    const cst_val *t;

    for (t=v; t; t=val_cdr(t))
	if (lvl_is_is_vowel(val_string(val_car(t))))
	    return TRUE;
    return FALSE;
}

static int lvl_is_has_vowel_in_syl(const cst_item *i)
{
    const cst_item *n;

    for (n=i; n; n=item_prev(n))
	if (lvl_is_is_vowel(item_feat_string(n,"name")))
	    return TRUE;
    return FALSE;
}

static int lvl_is_is_vowel(const char *p)
{
    /* this happens to work for icelandic SAMPA phoneset */
    if (strchr("aeiouOEYI",p[0]) == NULL)
	return FALSE;
    else
	return TRUE;
}

static int lvl_is_sonority(const char *p)
{
    /* A bunch of hacks for US English phoneset */
    if (lvl_is_is_vowel(p) || (lvl_is_is_silence(p)))
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

int lvl_is_syl_boundary(const cst_item *i,const cst_val *rest)
{
    // TODO adapt to is
    /* Returns TRUE if this should be a syllable boundary */
    /* This is of course phone set dependent              */
    int p, n, nn;

    if (rest == NULL)
	return TRUE;
    else if (lvl_is_is_silence(val_string(val_car(rest))))
	return TRUE;
    else if (!lvl_is_has_vowel_in_list(rest)) /* no more vowels so rest *all* coda */
	return FALSE;
    else if (!lvl_is_has_vowel_in_syl(i))  /* need a vowel */
	return FALSE;
    else if (lvl_is_is_vowel(val_string(val_car(rest))))
	return TRUE;
    else if (val_cdr(rest) == NULL)
	return FALSE;
    else 
    {   /* so there is following vowel, and multiple phones left */
	p = lvl_is_sonority(item_feat_string(i,"name"));
	n = lvl_is_sonority(val_string(val_car(rest)));
	nn = lvl_is_sonority(val_string(val_car(val_cdr(rest))));

	if ((p <= n) && (n <= nn))
	    return TRUE;
	else
	    return FALSE;
    }
}

static int lvl_is_lex_dist_to_vowel(const cst_val *rest)
{
    // TODO adapt to is
    if (rest == 0)
        return 0;  /* shouldn't get here */
    else if (lvl_is_is_vowel(val_string(val_car(rest))))
        return 0;
    else
        return 1+lvl_is_lex_dist_to_vowel(val_cdr(rest));
}


// cst_val *lvl_is_lex_lts_function(const struct lexicon_struct *l, 
//                                    const char *word, const char *pos)
// {
//     return NULL;
// }



cst_lexicon *lvl_is_lex_init(void)
{
    // /* Should it be global const or dynamic */
    // /* Can make lts_rules just a cart tree like others */
    // cst_lexicon *l;

    // l = cst_alloc(cst_lexicon,1);
    // l->name = "lvl_is_lex";

    // l->lts_function = lvl_is_lex_lts_function;
    if (lvl_is_lts_rules.name)
    return &lvl_is_lex;  /* Already initialized */

    lvl_is_lts_rules.name = "lvl_is";
    lvl_is_lts_rules.letter_index = lvl_is_lts_letter_index;

    lvl_is_lts_rules.models = lvl_is_lts_model;

    lvl_is_lts_rules.phone_table = lvl_is_lts_phone_table;
    lvl_is_lts_rules.context_window_size = 4;
    lvl_is_lts_rules.context_extra_feats = 1;
    lvl_is_lts_rules.letter_table = 0 /* lvl_is_lts_letter_table */;

    lvl_is_lex.name = "lvl_is";
    lvl_is_lex.num_entries = lvl_is_lex_num_entries;

    lvl_is_lex.data = (unsigned char *)(void *)lvl_is_lex_data;
    lvl_is_lex.syl_boundary = lvl_is_syl_boundary;

    lvl_is_lex.num_bytes = lvl_is_lex_num_bytes;
    lvl_is_lex.phone_table = (char **) lvl_is_lex_phone_table;
    lvl_is_lex.lts_rule_set = (cst_lts_rules *) &lvl_is_lts_rules;

    lvl_is_lex.phone_hufftable = lvl_is_lex_phones_huff_table;
    lvl_is_lex.entry_hufftable = lvl_is_lex_entries_huff_table;


    return &lvl_is_lex;

}
