//
// Fluid file routines for the Fast Light Tool Kit (FLTK).
//
// You may find the basic read_* and write_* routines to
// be useful for other programs.  I have used them many times.
// They are somewhat similar to tcl, using matching { and }
// to quote strings.
//
// Copyright 1998-2023 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include "io/Project_Writer.h"

#include "app/fluid.h"
#include "app/project.h"
#include "app/shell_command.h"
#include "app/undo.h"
#include "app/Fd_Snap_Action.h"

/// \defgroup flfile .fl Project File Operations
/// \{

using namespace fld;
using namespace fld::io;

/** \brief Write an .fl design description file.

 The .fl file format is documented in `fluid/README_fl.txt`.

 \param[in] filename create this file, and if it exists, overwrite it
 \param[in] selected_only write only the selected nodes in the widget_tree. This
    is used to implement copy and paste.
 \return 0 if the operation failed, 1 if it succeeded
 */
int fld::io::write_file(const char *filename, int selected_only, bool to_codeview) {
  Project_Writer out;
  return out.write_project(filename, selected_only, to_codeview);
}

// ---- Project_Writer ---------------------------------------------- MARK: -

/** \brief Construct local project writer. */
Project_Writer::Project_Writer()
: fout(NULL),
  needspace(0),
  write_codeview_(false)
{
}

/** \brief Release project writer resources. */
Project_Writer::~Project_Writer()
{
}

/**
 Open the .fl design file for writing.
 If the filename is NULL, associate stdout instead.
 \param[in] s the filename or NULL for stdout
 \return 1 if successful. 0 if the operation failed
 */
int Project_Writer::open_write(const char *s) {
  if (!s) {
    fout = stdout;
  } else {
    FILE *f = fl_fopen(s,"wb");
    if (!f) return 0;
    fout = f;
  }
  return 1;
}

/**
 Close the .fl design file.
 Don't close, if data was sent to stdout.
 \return 1 if succeeded, 0 if fclose failed
 */
int Project_Writer::close_write() {
  if (fout != stdout) {
    int x = fclose(fout);
    fout = stdout;
    return x >= 0;
  }
  return 1;
}

/** \brief Write an .fl design description file.
 \param[in] filename create this file, and if it exists, overwrite it
 \param[in] selected_only write only the selected nodes in the widget_tree. This
            is used to implement copy and paste.
 \param[in] sv if set, this file will be used by codeview
 \return 0 if the operation failed, 1 if it succeeded
 */
int Project_Writer::write_project(const char *filename, int selected_only, bool sv) {
  write_codeview_ = sv;
  undo_suspend();
  if (!open_write(filename)) {
    undo_resume();
    return 0;
  }
  write_string("# data file for the Fltk User Interface Designer (fluid)\n"
               "version %.4f",FL_VERSION);
  if(!g_project.include_H_from_C)
    write_string("\ndo_not_include_H_from_C");
  if(g_project.use_FL_COMMAND)
    write_string("\nuse_FL_COMMAND");
  if (g_project.utf8_in_src)
    write_string("\nutf8_in_src");
  if (g_project.avoid_early_includes)
    write_string("\navoid_early_includes");
  if (g_project.i18n_type) {
    write_string("\ni18n_type %d", g_project.i18n_type);
    switch (g_project.i18n_type) {
      case FD_I18N_NONE:
        break;
      case FD_I18N_GNU : /* GNU gettext */
        write_string("\ni18n_include"); write_word(g_project.i18n_gnu_include.c_str());
        write_string("\ni18n_conditional"); write_word(g_project.i18n_gnu_conditional.c_str());
        write_string("\ni18n_gnu_function"); write_word(g_project.i18n_gnu_function.c_str());
        write_string("\ni18n_gnu_static_function"); write_word(g_project.i18n_gnu_static_function.c_str());
        break;
      case FD_I18N_POSIX : /* POSIX catgets */
        write_string("\ni18n_include"); write_word(g_project.i18n_pos_include.c_str());
        write_string("\ni18n_conditional"); write_word(g_project.i18n_pos_conditional.c_str());
        if (!g_project.i18n_pos_file.empty()) {
          write_string("\ni18n_pos_file");
          write_word(g_project.i18n_pos_file.c_str());
        }
        write_string("\ni18n_pos_set"); write_word(g_project.i18n_pos_set.c_str());
        break;
    }
  }

  if (!selected_only) {
    write_string("\nheader_name"); write_word(g_project.header_file_name.c_str());
    write_string("\ncode_name"); write_word(g_project.code_file_name.c_str());
    g_layout_list.write(this);
    if (g_shell_config)
      g_shell_config->write(this);
    if (g_project.write_mergeback_data)
      write_string("\nmergeback %d", g_project.write_mergeback_data);
  }

  for (Fl_Type *p = Fl_Type::first; p;) {
    if (!selected_only || p->selected) {
      p->write(*this);
      write_string("\n");
      int q = p->level;
      for (p = p->next; p && p->level > q; p = p->next) {/*empty*/}
    } else {
      p = p->next;
    }
  }
  int ret = close_write();
  undo_resume();
  return ret;
}

/**
 Write a string to the .fl file, quoting characters if necessary.
 \param[in] w NUL terminated text
 */
void Project_Writer::write_word(const char *w) {
  if (needspace) putc(' ', fout);
  needspace = 1;
  if (!w || !*w) {fprintf(fout,"{}"); return;}
  const char *p;
  // see if it is a single word:
  for (p = w; is_id(*p); p++) ;
  if (!*p) {fprintf(fout,"%s",w); return;}
  // see if there are matching braces:
  int n = 0;
  for (p = w; *p; p++) {
    if (*p == '{') n++;
    else if (*p == '}') {n--; if (n<0) break;}
  }
  int mismatched = (n != 0);
  // write out brace-quoted string:
  putc('{', fout);
  for (; *w; w++) {
    switch (*w) {
    case '{':
    case '}':
      if (!mismatched) break;
    case '\\':
    case '#':
      putc('\\',fout);
      break;
    }
    putc(*w,fout);
  }
  putc('}', fout);
}

/**
 Write an arbitrary formatted word to the .fl file, or a comment, etc .
 If needspace is set, then one space is written before the string
 unless the format starts with a newline character \\n.
 \param[in] format printf style formatting string followed by a list of arguments
 */
void Project_Writer::write_string(const char *format, ...) {
  va_list args;
  va_start(args, format);
  if (needspace && *format != '\n') fputc(' ',fout);
  vfprintf(fout, format, args);
  va_end(args);
  needspace = !isspace(format[strlen(format)-1] & 255);
}

/**
 Start a new line in the .fl file and indent it for a given nesting level.
 \param[in] n indent level
 */
void Project_Writer::write_indent(int n) {
  fputc('\n',fout);
  while (n--) {fputc(' ',fout); fputc(' ',fout);}
  needspace = 0;
}

/**
 Write a '{' to the .fl file at the given indenting level.
 */
void Project_Writer::write_open() {
  if (needspace) fputc(' ',fout);
  fputc('{',fout);
  needspace = 0;
}

/**
 Write a '}' to the .fl file at the given indenting level.
 \param[in] n indent level
 */
void Project_Writer::write_close(int n) {
  if (needspace) write_indent(n);
  fputc('}',fout);
  needspace = 1;
}

/// \}
