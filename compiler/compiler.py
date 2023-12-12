#!/usr/bin/env python3

LOOP_TOP = -1 #whole program

_id = -1
def new_id():
    global _id
    _id = _id + 1
    return _id

indent_size = 4
indent = " " * indent_size

statements = ""
params = set()
vars = set()
def var(s):
    global vars
    global declarations
    if s not in vars:
        vars.add(s)
        declarations = declarations + indent + "int " + s + ";\n"
    return s

def param(p):
    global params
    if p not in params:
        params.add(p)
    return p

declarations = ""
def code(s):
    global statements
    statements = statements + indent + s + "\n"
    
class Expression(object):
    def __init__(self):
        raise NotImplementedError("Is abstract class")

class Seq(Expression):
    def __init__(self, children):
        self.children = children

    def generate(self):
        my_id = new_id()
        children_results = [c.generate() for c in self.children]

        eta_count = max([len(cr[1]) for cr in children_results])
        eta_vars = []
        loop_id_var = var("seq_" + str(my_id) + "_loop_id")
        my_loop_id = max([cr[0] for cr in children_results]) #assume loop id are in topological order
        code(loop_id_var + " = " + str(my_loop_id) + ";")
        for i in range(eta_count):
            eta_vars.append(var("seq_" + str(my_id) + "_eta_" + str(i)))

        eta_vars_code = [ev + " = 0" for ev in eta_vars]

        for cr in children_results:
            (child_loop_id, child_vars ) = cr
            for cv in range(eta_count):
                if cv < len(child_vars):
                    eta_vars_code[cv] += " + " + child_vars[cv]
                else:
                    eta_vars_code[cv] += " + " + child_vars[-1]
                
        for evc in eta_vars_code:
            code(evc + ";")

        
        return (my_loop_id, eta_vars)

class Alt(Expression):

    def __init__(self, left, right):
        self.left = left
        self.right = right

    def __init__(self, children):
        if len(children) == 1:
            self.left = children[0]
            self.right = Constant(LOOP_TOP, [0])
            return
        middle = len(children)//2

        left = children[0:middle]
        right = children[middle:]

        if len(left) == 1:
            self.left = left[0]
        else:
            self.left = Alt(left)
        
        if len(right) == 1:
            self.right = right[0]
        else:
            self.right = Alt(right)

    def helper(self, result_vars, big_vars, small_vars, last_expr=None, indent=0):
        if small_vars > big_vars:
            (small_vars, big_vars) = (big_vars, small_vars)
        if len(small_vars) == 0:
                for i in range(len(result_vars)):
                    bi = i
                    if bi >= len(big_vars):
                        bi = len(big_vars) - 1
                    if last_expr:
                        code(indent*" " + result_vars[i] + " = " + last_expr + ";")
                    else:
                        code(indent*" " + result_vars[i] + " = " + big_vars[bi] + ";")
                return
        code(indent*" " + "if (" + big_vars[0] + " > " + small_vars[0] + ") {")
        code((indent + indent_size)*" " + result_vars[0] + " = " + big_vars[0] + ";")
        self.helper(result_vars[1:], big_vars[1:], small_vars, big_vars[0], indent + indent_size)
        code(indent*" " + "} else {")
        code((indent + indent_size) *" " + result_vars[0] + " = " + small_vars[0] + ";")
        self.helper(result_vars[1:], big_vars, small_vars[1:], small_vars[0], indent + indent_size)
        code(indent*" " + "}")

    def generate(self):
        my_id = new_id()
        (left_loop_id, left_vars) = self.left.generate()
        (right_loop_id, right_vars) = self.right.generate()
        result_vars = []
        for i in range(len(left_vars) + len(right_vars) - 1):
            result_vars.append(var("alt_" + str(my_id) + "_eta_" + str(i)))
      
        self.helper(result_vars, left_vars, right_vars)

        return (max(left_loop_id, right_loop_id), result_vars)

class Loop(Expression):
    def __init__(self, body, loop_id, bound):
        self.body = body
        self.loop_id = loop_id
        self.bound = bound
    
    def generate(self):
        my_id = self.loop_id
        (body_loop_id, body_vars) = self.body.generate()
        
        if isinstance(self.bound, ParamBound):
            bound_var = var("loop_" + str(my_id) + "_bound")
            code(bound_var + " = " + self.bound.param_bound + ";")
        else:
            bound_var = var("loop_" + str(my_id) + "_bound")
            code(bound_var + " = " + str(self.bound.bound_value) + ";")

        if body_loop_id == self.loop_id:
            result_var = var("loop_" + str(my_id) + "_eta_0")
            for i in range(len(body_vars)):
                if i == 0:
                    code("if (" + str(bound_var) + " == " + str(i) + ") " + result_var + " = 0;")
                elif i < len(body_vars) - 1:
                    code("if (" + str(bound_var) + " == " + str(i) + ") " + result_var + " = (" + " + ".join(body_vars[0:i]) + ");")
                else:
                    code("if (" + str(bound_var) + " >= " + str(i) + ") " + result_var + " = (" + " + ".join(body_vars[0:-1]) + ") + " + body_vars[-1] + "*(" + bound_var + " - " + str(i) + ");")
            return (LOOP_TOP, [result_var])
        else:
            result_vars = []
            for i in range(len(body_vars)):
                result_vars.append(var("loop_" + str(my_id) + "_eta_" + str(i)))
            for i in range(len(body_vars) + 1):
                paquets = ""
                j = 0
                if i == 0:
                    while j < len(body_vars):
                        paquets += result_vars[j] + " = 0; "
                        j += 1
                else:
                    k = 0
                    if i == len(body_vars):
                        bound_expr = bound_var
                    else:
                        bound_expr = str(i)
                    while j < len(body_vars):
                        paquets += result_vars[k] + " = "
                        paquets += " + ".join(body_vars[j: j + i])
                        if (j + i) > len(body_vars):
                            paquets += " + " + body_vars[-1] + "*" + str((j + i) - len(body_vars))
                        if i == len(body_vars):
                            paquets += " + (" + bound_expr + " - " + str(i) + ")*" + body_vars[-1] 
                        paquets += "; "
                        j = j + i
                        k = k + 1
                    while k < len(body_vars):
                        paquets += result_vars[k] + " = " + body_vars[-1] + "*" + bound_expr + "; "
                        k = k + 1

                if i == len(body_vars):
                    cond = ">="
                else:
                    cond = "=="
                code("if (" + str(bound_var) + " " + cond + " " + str(i) + ") { " + paquets + "}")
            return (body_loop_id, result_vars)


class Constant(Expression):
    def __init__(self, loop_id, eta):
        self.eta = eta
        self.loop_id = loop_id

    def generate(self):
        my_id = new_id()
        code(var("cst_" + str(my_id) + "_loop_id") + " = " + str(self.loop_id) + ";")
        for i in range(len(self.eta)):
            code(var("cst_" + str(my_id) + "_eta_" + str(i)) + " = " + str(self.eta[i]) + ";")
        return (self.loop_id, ["cst_" + str(my_id) + "_eta_" + str(i) for i in range(len(self.eta))])

class Conditional(Expression):
    def __init__(self, test, tree):
        self.test = test
        self.tree = tree

    def generate(self):
        my_id = new_id()
        result = self.tree.generate()

        # compute the params
        for i in range(4):
            param_str = "param_" + str(i)
            if param_str in self.test:
                param(param_str)

        # compute the result
        loop_id = result[0]
        eta = result[1]
        eta_count = len(eta)
        for i in range(eta_count):
            code(eta[i] + " = (" + self.test + ") ? " + eta[i] + " : 0;")
        return (loop_id, eta)

class Bound(object):
    def __init__(self):
        raise NotImplementedError("Is abstract class")

class ParamBound(Bound):
    def __init__(self, param_bound):
        # compute the params
        for i in range(4):
            param_str = "param_" + str(i)
            if param_str in param_bound:
                param(param_str)

        self.param_bound = param_bound

class ConstantBound(Bound):
    def __init__(self, bound_value):
        self.bound_value = bound_value


def generate(formula):
    (loop_id, eta_vars) = formula.generate()
    code("return " + eta_vars[0] + ";")
    # Documentation print
    print("/*")
    print(" * WCET evaluation function")
    for p in sorted(list(params)):
        pnum = int(p.replace("param_",""))+1
        print(" * @param " + p + " ", pnum, "th procedure argument")
    print(" * @return The WCET of the procedure depending on its arguments")
    print(" */")
    # End of documentation
    print("int eval(", end="")
    first = True
    for p in sorted(list(params)):
        if not first:
            print(", ", end="")
        print("int " + p, end="")
        first = False
    print(") {\n" + declarations + "\n" + statements +  "}\n")

#audiobeam_find_max_in_arr = Alt([Constant(0, [1331, 1321, 500, 40, 30, 10]), Loop(Constant(2, [325, 265, 100, 50, 20]), 0, ParamBound(1))])
#audiobeam_find_max_in_arr = Seq([Constant(0, [1331, 1321]), Loop(Constant(2, [325, 265]), 2, ConstantBound(2))])
#generate(audiobeam_find_max_in_arr)

#toto = Loop(Seq([Constant(0, [20, 10]), Constant(0, [100])]), 0, ParamBound(1))
#generate(toto)




