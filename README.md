# CSI2P II Mini Project 1

## Introduction

Let's consider a CPU, which has 32 bits registers `r0`-`r255` and a 256 bytes memory.

In this project, you need to implement a binary expression calculator.

## Input

The input will contain several binary expressions consisting of integers, operators, parentheses, and three variables `x`, `y`, and `z`.

The following operators will appear in this project:

- `+`, `-`, `*`, `/`, `%`
- `=`
- `++`, `--` (including prefix and suffix, such as `x++`, `--y`, ... and so on)
- `+`, `-` (expressions such as `+x`, `-y`, ... and so on)
- others such as `>>`, `+=`, are unavailable and will not appear.

## Output

The output is a list of assembly codes. The instruction set architecture are listed in the table below.

If the input expressions contains illegal expression, you should handle it with the error handler. For the details, please refer to **Error Handler** below.

## Instruction Set Architecture

### Memory Operation

| Opcode | Operand1 | Operand2 | Meaning                                                    | Cycles |
| ------ | -------- | -------- | ---------------------------------------------------------- | ------ |
| load   | `reg`    | `[Addr]` | Load data in memory `[Addr]` and save into register `reg`. | 200    |
| store  | `[Addr]` | `reg`    | Store the data of register `reg` into memory `[Addr]`.     | 200    |

### Arithmetic Operation

| Opcode | Operand1 | Operand2 | Operand3 | Meaning                                          | Cycles |
| ------ | -------- | -------- | -------- | ------------------------------------------------ | ------ |
| add    | `rd`     | `rs1`    | `rs2`    | Perform `rs1+rs2` and save the result into `rd`. | 10     |
| sub    | `rd`     | `rs1`    | `rs2`    | Perform `rs1-rs2` and save the result into `rd`. | 10     |
| mul    | `rd`     | `rs1`    | `rs2`    | Perform `rs1*rs2` and save the result into `rd`. | 30     |
| div    | `rd`     | `rs1`    | `rs2`    | Perform `rs1/rs2` and save the result into `rd`. | 50     |
| rem    | `rd`     | `rs1`    | `rs2`    | Perform `rs1%rs2` and save the result into `rd`. | 60     |

- Note that both `rs1` and `rs2` can be a register or a value. However, `rd` must be a valid register.
- All operands should be separated by a space.
- **Important: Using the first 8 registers has no penalty. However, using other registers would double the instruction cycle.**
  - For example, `add r0 r1 r7` cost 10 cycles, while `add r8 r0 r23` cost 20 cycles.

## Variables

- The initial value of variables `x`, `y`, and `z` are stored in memory `[0]`, `[4]`, and `[8]` respectively. Before you use them, you have to load them into registers first.
- After the evaluation of the assembly code, the answer of the variables `x`, `y`, and `z` has to be stored in memory `[0]`, `[4]`, and `[8]` respectively.

## Grammar

Expression grammar for mini project 1.

Start with "statement".

Note that this only checks syntactical error such as "x++++y". However, semantic error like "5++" or "1=2+3" will pass the grammar.

```
tokens:
    END: end of line
    OP_ASSIGN:  "="  (Assign)
    OP_ADD:     "+"  (Add)
    OP_SUB:     "-"  (Sub)
    OP_MUL:     "*"  (Mul)
    OP_DIV:     "/"  (Div)
    OP_REM:     "%"  (Rem)
    OP_INC:     "++" (Inc)
    OP_DEC:     "--" (Dec)
    OP_PLUS:    "+"  (Plus)
    OP_MINUS:   "-"  (Minus)
    OP_MINUS:   "-"  (Minus)
    IDENTIFIER: xyz  (Variable)
    CONSTANT:   123  (Value)
    LPAR:       "("  (LPar)
    RPAR:       ")"  (RPar)

STMT
    → END
    | EXPR END
    ;
EXPR
    → ASSIGN_EXPR
    ;
ASSIGN_EXPR
    → ADD_EXPR
    | UNARY_EXPR OP_ASSIGN ASSIGN_EXPR
    ;
ADD_EXPR
    → MUL_EXPR
    | ADD_EXPR OP_ADD MUL_EXPR
    | ADD_EXPR OP_SUB MUL_EXPR
    ;
MUL_EXPR
    → UNARY_EXPR
    | MUL_EXPR OP_MUL UNARY_EXPR
    | MUL_EXPR OP_DIV UNARY_EXPR
    | MUL_EXPR OP_REM UNARY_EXPR
    ;
UNARY_EXPR
    → POSTFIX_EXPR
    | OP_INC UNARY_EXPR
    | OP_DEC UNARY_EXPR
    | OP_PLUS UNARY_EXPR
    | OP_MINUS UNARY_EXPR
    ;
POSTFIX_EXPR
    → PRI_EXPR
    | POSTFIX_EXPR OP_INC
    | POSTFIX_EXPR OP_DEC
    ;
PRI_EXPR
    → IDENTIFIER
    | CONSTANT
    | LPAR EXPR RPAR
    ;
```

## Error handler

If this expression could be compiled by GCC, it's a legal expression. Otherwise it's illegal.

Illegal expressions such as:

- x = 5++
- y = (((7/3)
- z = ++(y++)
- and all expressions that cannot pass GCC compilers should be handled by error handler.

When an error occurs, no matter how much your assembly has outputed, your output **must contain `Compile Error!` with newline**.

**Note that in our testcases, there won't be any undefined behavior expression.**

- You may check if an expression is undefined behavior by compiling a C program with `-Wall` flag. If it is, there should be some warnings that shows the word "undefined".

## Sample

### Sample Input 1

```c
x = z + 5
```

### Sample Output 1

```
load r0 [8]
add r0 r0 5
store [0] r0
```

Total cycle cost: 200(load) + 10(add) + 200(store) = 410 cycles.

### Sample Input 2

```c
x = (y++) + (++z)
z = ++(y++)
```

### Sample Output 2

```
Compile Error!
```

- Note that in sample 2, the first expression is correct, while the second one causes compile error.


