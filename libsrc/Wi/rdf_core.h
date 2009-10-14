/*
 *
 *  This file is part of the OpenLink Software Virtuoso Open-Source (VOS)
 *  project.
 *
 *  Copyright (C) 1998-2009 OpenLink Software
 *
 *  This project is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; only version 2 of the License, dated June 1991.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#ifndef __RDF_CORE_H
#define __RDF_CORE_H
#include "langfunc.h"
#include "sqlnode.h"
#include "rdf_mapping_jso.h"
#include "xmlparser.h" /* for xml_read_func_t and xml_read_abend_func_t */

#define IRI_TO_ID_IF_KNOWN	0 /*!< Return IRI_ID if known, integer zero (NULL) if not known or error is not NULL */
#define IRI_TO_ID_WITH_CREATE	1 /*!< Return IRI_ID if known or created on the fly, integer zero (NULL) if error is not NULL */
#define IRI_TO_ID_IF_CACHED	2 /*!< Return IRI_ID if known and is in cache, integer zero (NULL) if not known or known but not cached or error is not NULL */
extern caddr_t iri_to_id (caddr_t *qst, caddr_t name, int mode, caddr_t *err_ret);
extern caddr_t key_id_to_iri (query_instance_t * qi, iri_id_t iri_id_no);
extern int key_id_to_namespace_and_local (query_instance_t *qi, iri_id_t iid, caddr_t *subj_ns_ret, caddr_t *subj_loc_ret);
extern caddr_t rdf_type_twobyte_to_iri (short twobyte);
extern caddr_t rdf_lang_twobyte_to_string (short twobyte);
/*! \returns NULL for string, (ccaddr_t)((ptrlong)1) for unsupported, 2 for NULL, UNAME for others */
extern ccaddr_t xsd_type_of_box (caddr_t arg);
#define BNODE_IID_TO_LABEL_BUFFER(buf,iid) (((iid) >= MIN_64BIT_BNODE_IRI_ID) ? \
  sprintf (buf, "nodeID://b" BOXINT_FMT, (boxint)((iid)-MIN_64BIT_BNODE_IRI_ID)) : \
  sprintf (buf, "nodeID://" BOXINT_FMT, (boxint)(iid)) )
#define BNODE_IID_TO_LABEL(iid) (((iid) >= MIN_64BIT_BNODE_IRI_ID) ? \
  box_sprintf (30, "nodeID://b" BOXINT_FMT, (boxint)((iid)-MIN_64BIT_BNODE_IRI_ID)) : \
  box_sprintf (30, "nodeID://" BOXINT_FMT, (boxint)(iid)) )
#define BNODE_IID_TO_LABEL_LOCAL(iid) (((iid) >= MIN_64BIT_BNODE_IRI_ID) ? \
  box_sprintf (30, "b" BOXINT_FMT, (boxint)((iid)-MIN_64BIT_BNODE_IRI_ID)) : \
  box_sprintf (30, BOXINT_FMT, (boxint)(iid)) )
#define BNODE_IID_TO_TTL_LABEL_LOCAL(iid) (((iid) >= MIN_64BIT_BNODE_IRI_ID) ? \
  box_sprintf (30, "vb" BOXINT_FMT, (boxint)((iid)-MIN_64BIT_BNODE_IRI_ID)) : \
  box_sprintf (30, "v" BOXINT_FMT, (boxint)(iid)) )
#define BNODE_IID_TO_TALIS_JSON_LABEL(iid) (((iid) >= MIN_64BIT_BNODE_IRI_ID) ? \
  box_sprintf (30, "_:vb" BOXINT_FMT, (boxint)((iid)-MIN_64BIT_BNODE_IRI_ID)) : \
  box_sprintf (30, "_:v" BOXINT_FMT, (boxint)(iid)) )


/* Set of callback to accept the stream of RDF quads that are grouped by graph and share blank node IDs */

#define TRIPLE_FEED_NEW_GRAPH	0
#define TRIPLE_FEED_NEW_BLANK	1
#define TRIPLE_FEED_GET_IID	2
#define TRIPLE_FEED_TRIPLE	3
#define TRIPLE_FEED_TRIPLE_L	4
#define TRIPLE_FEED_COMMIT	5
#define TRIPLE_FEED_MESSAGE	6
#define COUNTOF__TRIPLE_FEED	7

typedef struct triple_feed_s {
  query_instance_t *tf_qi;
  id_hash_t *tf_blank_node_ids;
  caddr_t tf_app_env;		/*!< Environment for use by callbacks, owned by caller */
  const char *tf_input_name;	/*!< URI or file name or other name of source, can be NULL, owned by caller */
  caddr_t tf_default_graph_uri;	/*!< Default graph uri, owned by caller */
  caddr_t tf_current_graph_uri;	/*!< Currently active graph uri, owned by caller if equal to tf_default_graph_uri, local otherwise */
  caddr_t tf_base_uri;		/*!< Base URI to resolve relative URIs, owned by caller  */
  caddr_t tf_default_graph_iid;	/*!< Default graph iri ID, local */
  caddr_t tf_current_graph_iid;	/*!< Current graph iri ID, local */
  const char *tf_creator;	/*!< Name of BIF that created the feed (this name is printed in diagnostics) */
  ccaddr_t tf_cbk_names[COUNTOF__TRIPLE_FEED];	/*!< Callback names, owned by caller */
  query_t *tf_cbk_qrs[COUNTOF__TRIPLE_FEED];	/*!< Compiled callback queries, they can be NULLs for empty string names or names that starts with '!' */
  ptrlong tf_triple_count;	/*!< Number of triples that are sent to callbacks already, must be boxed before sending to SQL callbacks! */
  ptrlong tf_message_count;	/*!< Number of messages that are reported already, must be boxed before sending to SQL callbacks! */
  int *tf_line_no_ptr;		/*!< Pointer to some line number counter somewhere outside, may be NULL */
} triple_feed_t;

extern triple_feed_t *tf_alloc (void);
extern void tf_free (triple_feed_t *tf);
extern void tf_set_cbk_names (triple_feed_t *tf, const char **cbk_names);
extern void tf_new_graph (triple_feed_t *tf, caddr_t uri);
extern caddr_t tf_get_iid (triple_feed_t *tf, caddr_t uri);
extern void tf_commit (triple_feed_t *tf);
extern void tf_triple (triple_feed_t *tf, caddr_t s_uri, caddr_t p_uri, caddr_t o_uri);
extern void tf_triple_l (triple_feed_t *tf, caddr_t s_uri, caddr_t p_uri, caddr_t obj_sqlval, caddr_t obj_datatype, caddr_t obj_language);
extern void tf_report (triple_feed_t *tf, char msg_type, const char *sqlstate, const char *sqlmore, const char *descr);

#define TF_ONE_GRAPH_AT_TIME(tf) (NULL != (tf)->tf_cbk_names[TRIPLE_FEED_NEW_GRAPH])

#define TF_CHANGE_GRAPH(tf,new_uri) do { \
    if ((NULL != (tf)->tf_cbk_names[TRIPLE_FEED_NEW_GRAPH]) && (NULL != (tf)->tf_current_graph_uri)) \
      tf_commit ((tf)); \
    if ((tf)->tf_current_graph_uri != (tf)->tf_default_graph_uri) \
      dk_free_tree ((tf)->tf_current_graph_uri); \
    (tf)->tf_current_graph_uri = (new_uri); \
    (new_uri) = NULL; \
    if (TF_ONE_GRAPH_AT_TIME(tf)) { \
        dk_free_tree ((tf)->tf_current_graph_iid); \
        (tf)->tf_current_graph_iid = NULL; /* to avoid double free in case of error in tf_get_iid() below */ \
        (tf)->tf_current_graph_iid = tf_get_iid ((tf), (tf)->tf_current_graph_uri); \
        tf_new_graph ((tf), (tf)->tf_current_graph_uri); } \
  } while (0)

#define TF_CHANGE_GRAPH_TO_DEFAULT(tf) do { \
    if ((NULL != (tf)->tf_cbk_names[TRIPLE_FEED_NEW_GRAPH]) && (NULL != (tf)->tf_current_graph_uri)) \
      tf_commit ((tf)); \
    if ((tf)->tf_current_graph_uri != (tf)->tf_default_graph_uri) \
      dk_free_tree ((tf)->tf_current_graph_uri); \
    (tf)->tf_current_graph_uri = (tf)->tf_default_graph_uri; \
    if (NULL != (tf)->tf_cbk_names[TRIPLE_FEED_NEW_GRAPH]) { \
        dk_free_tree ((tf)->tf_current_graph_iid); \
        if (TF_ONE_GRAPH_AT_TIME(tf)) { \
            (tf)->tf_current_graph_iid = NULL; /* to avoid double free in case of error in tf_get_iid() below */ \
            (tf)->tf_default_graph_iid = tf_get_iid ((tf), (tf)->tf_default_graph_uri); } \
        (tf)->tf_current_graph_iid = box_copy ((tf)->tf_default_graph_iid); \
        tf_new_graph ((tf), (tf)->tf_current_graph_uri); } \
  } while (0)

#define TF_GRAPH_ARG(tf) ((TF_ONE_GRAPH_AT_TIME((tf))) ? &((tf)->tf_current_graph_iid) : &(tf->tf_current_graph_uri))

#define TTLP_STRING_MAY_CONTAIN_CRLF	0x0001	/*!< Single quoted and double quoted strings may contain newlines. */
#define TTLP_VERB_MAY_BE_BLANK		0x0002	/*!< Allows bnode predicates (but SPARQL processor may ignore them!) */
#define TTLP_ACCEPT_VARIABLES		0x0004	/*!< Allows variables, but triples with variables are ignored. */
#define TTLP_SKIP_LITERAL_SUBJECTS	0x0008	/*!< Allows literal subjects, but triples with them are ignored. */
#define TTLP_NAME_MAY_CONTAIN_PATH	0x0010	/*!< Allows '/', '#', '%' and '+' in local part of QName ("Qname with path") */
#define TTLP_ACCEPT_DIRTY_NAMES		0x0020	/*!< Allows ill bnode labels and invalid symbols between '<' and '>', i.e. in relative IRIs. */
#define TTLP_ACCEPT_DIRTY_SYNTAX	0x0040	/*!< Relax TURTLE syntax to include popular violations. */
#define TTLP_ERROR_RECOVERY		0x0080	/*!< Try to recover from lexical errors as much as it is possible. */
#define TTLP_ALLOW_TRIG			0x0100	/*!< Allows TriG syntax, thus loading data in more than one graph. */
#define TTLP_ALLOW_NQUAD		0x0200	/*!< Enables NQuads syntax but disables TURTLE and TriG */

#define TTLP_ALLOW_QNAME_A		0x01
#define TTLP_ALLOW_QNAME_HAS		0x02
#define TTLP_ALLOW_QNAME_IS		0x04
#define TTLP_ALLOW_QNAME_OF		0x08
#define TTLP_ALLOW_QNAME_THIS		0x10

typedef struct ttlp_s
{
  /* inputs */
  const char *ttlp_text;	/*!< Full source text, if short, or the beginning of long text, or an empty string */
  int ttlp_text_len;		/*!< Length of \c ttlp_text */
  int ttlp_text_ofs;		/*!< Current position in \c ttlp_text */
  xml_read_func_t ttlp_iter;
  xml_read_abend_func_t ttlp_iter_abend;
  void *ttlp_iter_data;
  encoding_handler_t *ttlp_enc;	/*!< Encoding of the source */
  long ttlp_flags;		/*!< Flags for dirty load */
  /* lexer */
  int ttlp_lexlineno;		/*!< Current line number */
  int ttlp_lexdepth;		/*!< Current number of not-yet-closed parenthesis */
  const char *ttlp_raw_text;	/*!< Raw text of the lexem */
  ptrlong ttlp_special_qnames;	/*!< Bitmask where every bit means that the identifier in qname, not a keyword */
  /* parser */
  const char *ttlp_err_hdr;	/*!< Human-readable phrase that gives a name to the parsing routine, e.g. "Turtle parser of web crawler" */
  caddr_t ttlp_catched_error;	/*!< The error that stopped the processing, as a three-element vector made by srv_make_new_error () */
  caddr_t ttlp_default_ns_uri;	/*!< IRI associated with ':' prefix */
  dk_set_t ttlp_namespaces;	/*!< get_keyword style list of namespace prefixes (keys) and IRIs (values) */
  dk_set_t ttlp_saved_uris;	/*!< Stack that keeps URIs. YACC stack is not used to let us free memory on error */
  dk_set_t ttlp_unused_seq_bnodes;	/*!< A list of bnodes that were allocated for use in lists but not used because lists are terminated before use */
  caddr_t ttlp_last_complete_uri;	/*!< Last \c QNAME or \c Q_IRI_REF that is expanded and resolved if needed */
  caddr_t ttlp_subj_uri;	/*!< Current subject URI, but it become object URI if ttlp_pred_is_reverse */
  caddr_t ttlp_pred_uri;	/*!< Current predicate URI */
  caddr_t ttlp_obj;		/*!< Current object URI or value */
  caddr_t ttlp_obj_type;	/*!< Current object type URI */
  caddr_t ttlp_obj_lang;	/*!< Current object language mark */
  int ttlp_pred_is_reverse;	/*!< Flag if ttlp_pred_uri is used as reverse, e.g. in 'O is P of S' syntax */
  caddr_t ttlp_formula_iid;	/*!< IRI ID of the blank node of the formula ( '{ ... }' notation of N3 */
  /* feeder */
  triple_feed_t *ttlp_tf;
} ttlp_t;


extern ttlp_t *ttlp_alloc (void);
extern void ttlp_free (ttlp_t *ttlp);

extern caddr_t rdf_load_turtle (
  caddr_t text_or_filename, int arg1_is_filename, caddr_t base_uri, caddr_t graph_uri, long flags,
  ccaddr_t *cbk_names, caddr_t app_env,
  query_instance_t *qi, wcharset_t *query_charset, caddr_t *err_ret );

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

extern int ttlyyparse (ttlp_t *ttlp_arg, yyscan_t scanner);
extern int nqyyparse (ttlp_t *ttlp_arg, yyscan_t scanner);
extern void ttlyyerror_impl (ttlp_t *ttlp_arg, const char *raw_text, const char *strg);
extern void ttlyyerror_impl_1 (ttlp_t *ttlp_arg, const char *raw_text, int yystate, short *yyssa, short *yyssp, const char *strg);

extern ptrlong ttlp_bit_of_special_qname (caddr_t qname);
extern caddr_t DBG_NAME (ttlp_expand_qname_prefix) (DBG_PARAMS ttlp_t *ttlp_arg, caddr_t qname);
extern caddr_t DBG_NAME (tf_bnode_iid) (DBG_PARAMS triple_feed_t *tf, caddr_t boxed_sparyytext);
extern caddr_t DBG_NAME (tf_formula_bnode_iid) (DBG_PARAMS ttlp_t *ttlp_arg, caddr_t boxed_sparyytext);
#ifdef MALLOC_DEBUG
#define ttlp_expand_qname_prefix(ttlp,qname) DBG_NAME (ttlp_expand_qname_prefix) (__FILE__, __LINE__, (ttlp), (qname))
#define tf_bnode_iid(tf, boxed_sparyytext) DBG_NAME (tf_bnode_iid) (__FILE__, __LINE__, (tf), (boxed_sparyytext))
#define tf_formula_bnode_iid(ttlp,boxed_sparyytext) DBG_NAME (tf_formula_bnode_iid) (__FILE__, __LINE__, (ttlp), (boxed_sparyytext))
#endif
extern caddr_t ttlp_uri_resolve (ttlp_t *ttlp_arg, caddr_t qname);

/* Numeric values of these constants are important, do not alter them. Theyh're used in tricky way. */
#define TTLP_STRLITERAL_QUOT 		1
#define TTLP_STRLITERAL_QUOT_AT		2
#define TTLP_STRLITERAL_3QUOT 		3
#define TTLP_STRLITERAL_3QUOT_AT	4
extern caddr_t ttlp_strliteral (ttlp_t *ttlp_arg, const char *sparyytext, int mode, char delimiter);
extern caddr_t ttl_query_lex_analyze (caddr_t str, wcharset_t *query_charset);

extern void ttlp_triple_and_inf (ttlp_t *ttlp_arg, caddr_t o_uri);
extern void ttlp_triple_l_and_inf (ttlp_t *ttlp_arg, caddr_t o_sqlval, caddr_t o_dt, caddr_t o_lang);

#define RDFXML_COMPLETE		0
#define RDFXML_OMIT_TOP_RDF	1
#define RDFXML_IN_ATTRIBUTES	2

extern void
rdfxml_parse (query_instance_t * qi, caddr_t text, caddr_t *err_ret,
  int mode_bits, const char *source_name, caddr_t base_uri, caddr_t graph_uri,
  ccaddr_t *stmt_texts, caddr_t app_env,
  const char *enc, lang_handler_t *lh
   /*, caddr_t dtd_config, dtd_t **ret_dtd,
   id_hash_t **ret_id_cache, xml_ns_2dict_t *ret_ns_2dict*/ );

/* Metadata about free-text index on DB.DBA.RDF_OBJ */
extern id_hash_t *rdf_obj_ft_rules;

extern int uriqa_dynamic_local;
extern caddr_t uriqa_get_host_for_dynamic_local (query_instance_t *qi, int * is_https);
extern caddr_t uriqa_get_default_for_connvar (query_instance_t *qi, const char *varname);

#define RDF_GRAPH_PERM_READ 0x01
#define RDF_GRAPH_PERM_WRITE 0x02
#define RDF_GRAPH_PERM_SPONGE 0x04
#define RDF_GRAPH_PERM_LIST 0x08
#define RDF_GRAPH_PERM_DEFAULT (RDF_GRAPH_PERM_READ | RDF_GRAPH_PERM_WRITE | RDF_GRAPH_PERM_SPONGE | RDF_GRAPH_PERM_LIST)

extern id_hash_t *rdf_graph_group_dict_htable;			/*!< Dictionary of graph group members: group IID is key, vector of member IIDs is value */
extern id_hash_iterator_t *rdf_graph_group_dict_hit;	/*!< Hash iterator for \c rdf_graph_group_dict_htable */
extern id_hash_t *rdf_graph_public_perms_dict_htable;		/*!< Dictionary of public permissions for graphs: graph/group IID is key, copy of DB.DBA.RDF_GRAPH_USER.RGU_PERMISSIONS is a value */
extern id_hash_iterator_t *rdf_graph_public_perms_dict_hit;	/*!< Hash iterator for \c rdf_graph_group_dict_htable */
extern id_hash_t *rdf_graph_group_of_privates_dict_htable;		/*!< Dictionary of private graphs, to accelerate the access to content of virtrdf:PrivateGraphs */
extern id_hash_iterator_t *rdf_graph__of_privates_dict_hit;	/*!< Hash iterator for \c rdf_graph_group_of_privates_dict_htable */
extern id_hash_t *rdf_graph_default_world_perms_of_user_dict_htable;		/*!< Dictionary of default permissions for users: user ID is key, copy of DB.DBA.RDF_GRAPH_USER.RGU_PERMISSIONS for #0 is a value */
extern id_hash_iterator_t *rdf_graph_default_world_perms_of_user_dict_hit;	/*!< Hash iterator for \c rdf_graph_default_world_perms_of_user_dict_htable */
extern id_hash_t *rdf_graph_default_private_perms_of_user_dict_htable;		/*!< Dictionary of default permissions for users: user ID is key, copy of DB.DBA.RDF_GRAPH_USER.RGU_PERMISSIONS for #0 is a value */
extern id_hash_iterator_t *rdf_graph_default_private_perms_of_user_dict_hit;	/*!< Hash iterator for \c rdf_graph_default_private_perms_of_user_dict_htable */
extern id_hash_t *rdf_graph_default_private_perms_of_user_dict_htable;		/*!< Dictionary of default permissions for users: user ID is key, copy of DB.DBA.RDF_GRAPH_USER.RGU_PERMISSIONS for #0 is a value */
extern id_hash_iterator_t *rdf_graph_default_private_perms_of_user_dict_hit;	/*!< Hash iterator for \c rdf_graph_default_private_perms_of_user_dict_htable */

extern caddr_t boxed_zero_iid;
extern caddr_t boxed_8k_iid;
extern caddr_t boxed_nobody_uid;
#endif
