#ifndef POLYNOMIALS_PARSER_C
#define POLYNOMIALS_PARSER_C

#include "parser.h"

#define DEF_ALLOC_SIZE 32
#define DEF_ALLOC_COEFF 2

/**
 * czy może tak być że ReadLine skończy wczytywać przed EOF i następne wywołanie
 * się scrashuje
 */
static void CheckAlloc(const void *pocharer) {
  if (!pocharer)
    exit(1);
}

static Mono *AllocMemForMonos(size_t count) {
  Mono *res = calloc(count, sizeof(*res));
  CheckAlloc(res);
  return res;
}

static Mono *IncreaseMemForMonos(Mono *monos, size_t *size) {
  *size *= DEF_ALLOC_COEFF;
  Mono *res = realloc(monos, *size * sizeof *res);
  CheckAlloc(res);
  return res;
}

poly_coeff_t StrToCoeff(char **string) {
  assert(string && *string);
  return strtol(*string, string, 10);
}

poly_exp_t StrToExp(char **string) {
  assert(string && *string);
  return strtol(*string, string, 10);
}

static Poly StrToPoly(char **string);

static Mono StrToMono(char **string) {
  //assert(string && *string && **string == '(');
  Mono res;
  (*string)++;
  res.p = StrToPoly(string);
  assert(**string == ',');
  (*string)++;
  assert((**string >= '0' && **string <= '9') || **string == '-');
  res.exp = StrToExp(string);
  assert(**string == ')');
  (*string)++;
  return res;
}
/**
 *
 **/
static Poly StrToPoly(char **string) {
  assert(string && *string);
  Poly res;
  if (**string == '(') {
    size_t size = 1, i = 0;
    Mono *monos = AllocMemForMonos(size);
    while (**string != '\0' && **string != ',') {
      assert(i == 0 || **string == '+');
      if (i > 0) {
        (*string)++;
        if (**string == '\0')
          break;
      }
      if (i == size)
        monos = IncreaseMemForMonos(monos, &size);
      monos[i++] = StrToMono(string);
    }
    if (i > 1) {
      res = PolyAddMonos(i, monos);
      free(monos);
    }
    else {
      assert(i == 1); // nie dostaje pustych linii (sam '+' ?)
      res = PolyFromMonos(monos, i);
      res = PolyDelZeros(&res);
    }
    res = PolySimplify(&res);
  } else {
    assert((**string >= '0' && **string <= '9') || **string == '-');
    res = PolyFromCoeff(StrToCoeff(string));
  }
  return res;
}


/*
static Line AllocMemForLine() {
  Line res;
  res.chars = calloc(DEF_ALLOC_SIZE, sizeof res.chars);
  CheckAlloc(res.chars);
  res.size = DEF_ALLOC_SIZE;
  res.type = EMPTY_LINE;
  res.last_index = 0;
  res.is_correct = true;
  res.is_eof = false;
  return res;
}

static void IncreaseLineSize(Line *line) {
  line->size *= DEF_ALLOC_COEFF;
  line->chars = realloc(line->chars, sizeof(line->chars) * line->size);
  CheckAlloc(line->chars);
}
*/

static bool IsNumber(char ch) {
  return ch >= '0' && ch <= '9';
}

static deg_by_arg_t ReadDegByArg(Line *line, size_t arg_i) {
  assert(line->error_type = NONE_ERR);
  deg_by_arg_t res;
  errno = 0;
  char *endptr = NULL;
  res = strtoul((char *) &line->chars[arg_i], &endptr, 10);
  if (errno == ERANGE || *endptr != '\0')
    line->error_type = WR_DEG_BY_VAR;
  return res;
}

static at_arg_t ReadAtArg(Line *line, size_t arg_i) {
  assert(line->error_type = NONE_ERR);
  at_arg_t res;
  errno = 0;
  char *endptr = NULL;
  res = strtol((char *) &line->chars[arg_i], &endptr, 10);
  if (errno == ERANGE || *endptr != '\0')
    line->error_type = WR_AT_VAL;
  return res;
}

static Command ReadCommand(Line *line) {
  assert(line->type == OPER);
  Command res;
  res.op = NONE_OP;
  char *oper = (char *) line->chars;
  if (line->last_index == 3 && oper[0] == 'A' && oper[1] == 'D' && oper[2] == 'D')
    res.op = ADD;
  else if (line->last_index == 3 && oper[0] == 'M' && oper[1] == 'U' && oper[2] == 'L')
    res.op = MUL;
  else if (line->last_index == 3 && oper[0] == 'N' && oper[1] == 'E' && oper[2] == 'G')
    res.op = NEG;
  else if (line->last_index == 3 && oper[0] == 'S' && oper[1] == 'U' && oper[2] == 'B')
    res.op = SUB;
  else if (line->last_index == 3 && oper[0] == 'D' && oper[1] == 'E' && oper[2] == 'G')
    res.op = DEG;
  else if (line->last_index == 3 && oper[0] == 'P' && oper[1] == 'O' && oper[2] == 'P')
    res.op = POP;
  else if (line->last_index == 4 && oper[0] == 'Z' && oper[1] == 'E' && oper[2] == 'R' && oper[3] == 'O')
    res.op = ZERO;
  else if (line->last_index >= 5 && oper[0] == 'I' && oper[1] == 'S' && oper[2] == '_') {
    if (line->last_index == 8 && oper[3] == 'C' && oper[4] == 'O' && oper[5] == 'E' && oper[6] == 'F' && oper[7] == 'F')
      res.op = IS_COEFF;
    else if (line->last_index == 7 && oper[3] == 'Z' && oper[4] == 'E' && oper[5] == 'R' && oper[6] == 'O')
      res.op = IS_ZERO;
    else if (line->last_index == 5 && oper[3] == 'E' && oper[4] == 'Q')
      res.op = IS_EQ;
    else
      line->error_type = WR_COMMAND;
  }
  else if (line->last_index >= 5 && oper[0] == 'D' && oper[1] == 'E' && oper[2] == 'G' && oper[3] == '_' && oper[4] == 'B' && oper[5] == 'Y') {
    if (oper[6] == ' ' && IsNumber(oper[7])) {
      res.op = DEG_BY;
      res.deg_by_arg = ReadDegByArg(line, 7);
    }
    else
      line->error_type = WR_DEG_BY_VAR;
  }
  else if (line->last_index >= 1 && oper[0] == 'A' && oper[1] == 'T') {
    if (line->last_index >= 3 && oper[2] == ' ' && (oper[3] == '-' || IsNumber(oper[3]))) {
      res.op = AT;
      res.at_arg = ReadAtArg(line, 3);
    } else
      line->error_type = WR_AT_VAL;
  }
  else if (line->last_index == 5 && oper[0] == 'P' && oper[1] == 'R' && oper[2] == 'I' && oper[3] == 'N' && oper[4] == 'T')
    res.op = PRINT;
  else if (line->last_index == 5 && oper[0] == 'C' && oper[1] == 'L' && oper[2] == 'O' && oper[3] == 'N' && oper[4] == 'E')
    res.op = CLONE;
  else
    line->error_type = WR_COMMAND;
  return res;
}
/*
static bool IsCorrectOperChar(char ch) {
  return (IsNumber(ch) || (ch >= 'A' && ch <= 'Z') || ch == '-' || ch == '_');
}
*/
static bool IsOperLine(int ch) {
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

static bool IsIgnoredLine(int ch) {
  return ch == '#' || ch == EOF || ch =='\n';
}
static bool IsCorrectPolyChar(char last_ch, char ch) {
  bool res = true;
  if (last_ch == '(' && ch != '(' && !IsNumber(ch) && ch != '-')
    res = false;
  else if (last_ch == ',' && !IsNumber(ch) && ch != '-')
    res = false;
  else if (last_ch == ')' && ch != ',' && ch != '+')
    res = false;
  else if (last_ch == '-' && !IsNumber(ch))
    res = false;
  else if (IsNumber(last_ch) && !IsNumber(ch) && ch != ',' && ch != ')')
    res = false;
  else if (last_ch == '+' && ch != '(')
    res = false;
  return res;
}

// akceptuje wiodące zera
static bool IsCorrectPoly(char *str, size_t str_size) {
  assert(str && str_size > 0);
  bool is_correct = true;
  if (IsNumber(str[0]) || (str[0] == '-' && str_size > 1)) {
    for (size_t i = 1; is_correct && i < str_size; i++)
      if (!IsNumber(str[i]))
        is_correct = false;
  }
  else if (str[0] == '(') {
    int parenths = 1;
    size_t cl_parenths = 0, commas = 0;
    for (size_t i = 1 ; is_correct && i < str_size; i++) {
      if (parenths < 0 || !IsCorrectPolyChar(str[i - 1], str[i]))
        is_correct = false;
      if (str[i] == '(') parenths++;
      else if (str[i] == ')') {
        cl_parenths++;
        parenths--;
      } else if (str[i] == ',')
        commas++;
    }
    if (parenths != 0 || cl_parenths != commas || str[str_size - 1] == '+')
      is_correct = false;
  }
  else is_correct = false;
  return is_correct;
}
/*
static Poly ConvertLineToPoly(char *str, size_t str_size) {
  if (IsCorrectPoly(str, str_size) && HasCorrectNumbers((char *) str, str_size)) {
    line->p = StrToPoly(&str);
  }
  else line->error_type = WR_POLY;
}
*/

static Line NewLine() {
  Line res;
  res.chars = malloc(DEF_ALLOC_SIZE * sizeof(*res.chars));
  res.size = DEF_ALLOC_SIZE;
  CheckAlloc(res.chars);
  res.last_index = 0;
  res.error_type = NONE_ERR;
  res.is_eof = false;
  return res;
}

static void IncreaseLineSize(Line *line) {
  line->chars = realloc(line->chars, DEF_ALLOC_COEFF * line->size * sizeof(*line->chars));
  CheckAlloc(line->chars);
  line->size *= DEF_ALLOC_COEFF;
}

static Line ReadLine() {
  Line res;
  int ch = getchar();
  if (IsIgnoredLine(ch)) {
    res.type = EMPTY;
    res.error_type = NONE_ERR;
    while (ch!= EOF && ch != '\n')
      ch = getchar();
  }
  else {
    res = NewLine();
    if (IsOperLine(ch))
      res.type = OPER;
    else res.type = POLY;
    res.chars[res.last_index++] = (char) ch;
    while ((ch = getchar()) != EOF && ch != '\n') {
      res.chars[res.last_index++] = (char) ch;
      if (res.last_index == res.size)
        IncreaseLineSize(&res);
    }
    res.chars[res.last_index] = '\0';
  }
  res.is_eof = ch == EOF ? true : false;
  return res;
}

static bool HasCorrectNumbers(char *str, size_t str_size) {
  errno = 0;
  bool correct = true;
  if (IsNumber(str[0]) || str[0] == '-') {
    strtol(str, NULL, 10);
    if (errno == ERANGE)
      correct = false;
  }
  for (size_t i = 0; correct && i < str_size - 1; i++) {
    if (str[i] == '(' && (str[i + 1] == '-' || IsNumber(str[i + 1]))) {
      strtol(&str[i + 1], NULL, 10);
      if (errno == ERANGE)
        correct = false;
    }
    else if (str[i] == ',') {
      long exp = strtol(&str[i + 1], NULL, 10);
      if (exp > INT_MAX || exp < 0) {
        correct = false;
      }
    }
  }
  return correct;
}

Line GetNextLine() {
  Line res = ReadLine();
  char *str = res.chars;
  if (res.type == POLY) {
    if (IsCorrectPoly(str, res.last_index) && HasCorrectNumbers((char *) str, res.last_index))
      res.p = StrToPoly((char **) &str);
    else
      res.error_type = WR_POLY;
  }
  else if (res.type == OPER)
    res.c = ReadCommand(&res);
  //else res.
  //if (res.error_type != NONE_ERR)
    //FreeUncorrectLine(&res);
  return res;
}

#endif //POLYNOMIALS_POLY_PARSER_C