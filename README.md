* # the development log

## error handling

### target function

- detect the error when compiling, including grammar errors and syntax error
- need line information 

### unreleased 

- a signal table for each indent
- a processer for Syntax error

### 2022-10-15:

#### Added:

- finish the trigger and the print function of the error processor
- For the components that may have errors, it will return a number that means correct if it is 0 and otherwise, so that its uppper layer will add error. 

### 2022-10-14:

#### Added:

- checking the grammar error (lexical error) like missing `;`, `]`, `)` etc. when lexer working

### 2022-10-13:

#### Changed:

- Add line attribute to the ast who may encounter errors

### 2022-10-11：

### Added:

- a error handing class

#### Changed:

- change the way of pre-fetching in order to avoid missing `;` error

### 2022-10-10:

#### Added:

- add a line information when parsing
- a signal table for checking 

#### Changed:

- hide the print result from the parser
