
#ifndef GETOPT_H
#define GETOPT_H

// long options
#define null_argument 0     /*Argument Null*/
#define no_argument 0       /*Argument Switch Only*/
#define required_argument 1 /*Argument Required*/
#define optional_argument 2 /*Argument Optional*/

// short Options
#define ARG_NULL 0 /*Argument Null*/
#define ARG_NONE 0 /*Argument Switch Only*/
#define ARG_REQ 1  /*Argument Required*/
#define ARG_OPT 2  /*Argument Optional*/

#ifdef __cplusplus
#define GO_THROW throw()
extern "C"
{
#else
#define GO_THROW
#endif
  extern int optind;
  extern int opterr;
  extern int optopt;

  struct opti_a
  {
    const char *name;
    int has_arg;
    int *flag;
    int val;
  };
  extern char *opta_a;
  extern int getopt_a(int argc, char *const *argv, const char *optstring) GO_THROW;
  extern int getopt_long_a(int argc, char *const *argv, const char *options, const struct opti_a *long_options, int *opt_index) GO_THROW;
  extern int getopt_long_only_a(int argc, char *const *argv, const char *options, const struct opti_a *long_options, int *opt_index) GO_THROW;

  struct opti_w
  {
    const wchar_t *name;
    int has_arg;
    int *flag;
    int val;
  };
  extern wchar_t *opta_w;
  extern int getopt_w(int argc, wchar_t *const *argv, const wchar_t *optstring) GO_THROW;
  extern int getopt_long_w(int argc, wchar_t *const *argv, const wchar_t *options, const struct opti_w *long_options, int *opt_index) GO_THROW;
  extern int getopt_long_only_w(int argc, wchar_t *const *argv, const wchar_t *options, const struct opti_w *long_options, int *opt_index) GO_THROW;

#ifdef __cplusplus
}
#endif

#undef GO_THROW

#ifdef _UNICODE
#define getopt getopt_w
#define getopt_long getopt_long_w
#define getopt_long_only getopt_long_only_w
#define option opti_w
#define optarg opta_w
#else
#define getopt getopt_a
#define getopt_long getopt_long_a
#define getopt_long_only getopt_long_only_a
#define option opti_a
#define optarg opta_a
#endif
#endif // GETOPT_H
