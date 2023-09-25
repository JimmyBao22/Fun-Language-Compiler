# Fun-Language-Compiler

"Fun", a lightweight and Python-inspired language with support for conditionals, functions, recursion, arrays, and co-routines. Below are some examples:

## Conditionals
```python
integer n = 90

if (n % 2 == 0) {
    print("The number is even.")
} else {
    print("The number is odd.")
}
```

## For Loops

```python
integer f[n+2]
f[0] = 0
f[1] = 1
for(integer i = 2; i<=n; i = i + 1){
    if[i] = f[i-1] + f[i-2]
}
print(f[n])
```

## While Loops
```python
boolean test = true
integer i = 0

while(test) {
    i = i + 1
    
    if (i > 10) {
        test = !test
    }
}
```

## Functions

```python
integer[10] array

array[0] = 10
array[1] = 32
array[2] = 100
array[3] = 1231
array[4] = 43242
array[5] = 342242
array[6] = 2343244
array[7] = 2888899
array[8] = 28888990
array[9] = 909009000

fun binarySearchRecursive(key) {
    integer last = 9
    integer first = 0

    if (last>=first){  
        int mid = first + (last - first) / 2;  

        if (arr[mid] == key ){  
            return true;  
        }  
        if (arr[mid] > key) {  
            return binarySearch(arr, first, mid-1, key)
        } else {  
            return binarySearch(arr, mid+1, last, key)
        }  
    }  

    return false;  
}
```

# Fun Programming Language Specification

## OPERATIONS AND PRECEDENCE
Operations higher in the following table have higher precedence than any operation below it. Operations at the same row in the list have equal precedence and are evaluated left to right. The operations are placed within curly braces in the below table but do not include them.

Let a and b be expressions that evaluate to unsigned 64 bit integers:

Operation

**{(a)}**
**{!a}**
**{a * b} , {a / b}, {a % b}**
**{a + b}, {a - b}**
**{a < b}, {a <= b}, {a > b}, {a >= b}**
**{a == b}, {a != b}**
**{a && b}**
**{a || b}**
--------
Every operation in this table returns an unsigned 64 bit integer. Every operation included in the previous spec has the exact same behavior.
The relational operators return 0 if the relation is false, and 1 if it is true.

(a) returns the value of a.

The logical operators (&&, ||, !) work like their counterparts in C. Non-zero operands are treated as “true”, and zero is treated as “false”. If the operation has a value of true, 1 is returned, and if the operation has a value of false, 0 is returned.

The operands of every binary operation are evaluated left to right.

Short-circuiting is NOT implemented. If the first operand of && is false, the second operand IS STILL evaluated. If the first operand of || is true, the second operand IS STILL evaluated.

/ and % by a zero second operand has undefined behavior.

## VARIABLES
Variables must abide by the naming guidelines of the first spec.

Variables names may not have the following names:

**Invalid Names**
**if**
**else**
**while**
**return**
**fun**
**-------**
Variables are declared, assigned, and reassigned by:
```python
<variableName> = <expression>
```
Variables defined outside of a function are global.

The use of undefined variables in expression has undefined behavior.

## FUNCTIONS
Syntax For Defining A Function
```python
fun <functionName>(<parameter1>, <parameter2>, ..., <parameterN>) {
    <statement1>
    <statement2>
    ...
    <statementK>
}
```

### Syntax For Calling a Function
```python
<functionName>(<argument1>, <argument2>, ..., <argumentN>)
```
### Requirements
- May have multiple parameters separated by commas
- May have no parameters
- Cannot have parameters of the same name.
- The arguments to functions must be expressions that evaluate to an unsigned 64 bit integer.
- Every function returns a value. If a return statement is never hit in the function, the function returns 0.
- A return statement is written as: return <expression>
- Function calls by themselves do constitute a statement. So a call to a function f, f(1), for example, following another valid statement will not result in an error. Note that a literal like 40 would.
- Function calls whose number of arguments doesn't match the number of parameters the corresponding function has will result in an error.
The arguments of a function call are evaluated from left to right.
- Functions have a local scope, which consists of the parameters and any variables declared within the function
  - If a local variable has the same name as a global variable, any instance of the variable’s name in the function is considered to refer to the local variable.
  - Any variable declared within a function is considered local.
    - When an assignment statement is reached in a function:
      - If the LHS is a local variable
        - Reassign local variable
      - Else If the LHS is a global variable
        - Reassign global variable
      - Else
        - Make new local variable
- If there is a global variable and a parameter with the same name, any identifiers of that name are considered to refer to the parameter
- Functions CAN be recursive.
- If function 1 makes a call to function 2, function2 must have been defined before function 1 is called.
- Variables and Functions CAN Share names
- Functions CANNOT be redefined.
- Functions CANNOT be defined inside of other functions.
- Undefined functions CANNOT be called.
- Function names follow the exact same naming rules as variable names, defined in the previous spec, along with the restriction of not using special keywords that was introduced earlier in this spec. Additionally, functions may not be named “print”.

## IF STATEMENTS AND WHILE LOOPS
### If Statement Syntax
```python
if (<expression>) {
    <statement1>
    <statement2>
    ...
    <statementK>
}
```
### If Statement With Else Clause Syntax
```python
if (<expression>) {
    <statement1>
    <statement2>
    ...
    <statementK>
} else {
    <statementK+1>
    <statementK+2>
    ...
    <statementK+N>
}
```
### While Loop Syntax
```python
while (<expression>) {
    <statement1>
    <statement2>
    ...
    <statementN>
}
```
### Requirements
- A condition of 0 represents false, and the corresponding if / while body doesn’t run. If a condition is non zero, then the corresponding body does run.
- An else clause cannot appear without a corresponding preceding if clause.
- The body of if statements and while loops do not get their own scope separate from the global scope.

## ERRORS AND UNDEFINED BEHAVIOUR
Fun code that does not abide by the spec has undefined behavior.

Additionally, having statements/function declarations that span multiple lines or share the same line is also undefined behavior. Basically, new lines separate statements/fun decs from each other.

##P RINT
### Print Syntax
```python
print(<expression>)
```
and it prints the unsigned 64 bit value of the expression passed to it.

## ADDITIONAL NOTES
- The code is a sequence of statements. Statements include print, if, while, variable assignment, function declarations, function calls. Expressions by themselves are not statements and should not appear except as part of a statement.
  - f(5) is a valid statement (function call)
  - 5/2 is not a valid statement (it's just an expression)
  - x = 5/2 is a valid statement (variable assignment)
  - f(5) % 5 is not a valid statement (it's just an expression)
