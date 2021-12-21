#include <functional>
#include <iostream>
#include <list>

#include "surelog.h"

// UHDM
#include <uhdm/ElaboratorListener.h>
#include <uhdm/uhdm.h>
#include <uhdm/vpi_listener.h>

//#define DEBUG 1
#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

// functions declarations
std::string visitbit_sel(vpiHandle, bool);
std::string visitpart_sel(vpiHandle, bool);
std::string visithier_path(vpiHandle);
std::string visitindexed_part_sel(vpiHandle);
std::string visitOperation(vpiHandle);
void visitTernary(vpiHandle);
void visitBlocks(vpiHandle);
std::string visitCond(vpiHandle, bool);
void visitTopModules(vpiHandle);
void visitAssignment(vpiHandle);
void findTernaryInOperation(vpiHandle);


// global variables
bool saveVariables = false;
std::list <std::string> save, nets, ternaries, cases, ifs;

void print_list(std::list<std::string> &list) {
    for (auto const &i: list) {
        std::cout << i << std::endl;
    }
    return;
}

std::string visitref_obj(vpiHandle h, bool fullName = true) {
  std::string out = "";
  vpiHandle actual = vpi_handle(vpiActual, h);
  fflush(stdout);
  if(actual) {
    //out += "REF_OBJ Type: ";
    //out += std::to_string(((const uhdm_handle *)actual)->type);
    fflush(stdout);
    if(((const uhdm_handle *)actual)->type == UHDM::uhdmenum_const) {
      out += "/*ENUM*/";
      s_vpi_value value;
      vpi_get_value(actual, &value);
      out += value.format == 0
        ? (fullName ? vpi_get_str(vpiFullName, actual) : vpi_get_str(vpiName, actual))
        : std::to_string(value.value.integer);
    }
    else if(((const uhdm_handle *)actual)->type == UHDM::uhdmparameter) {
      out += "/*PAR*/";
      s_vpi_value value;
      vpi_get_value(actual, &value);
      out += value.format == 0
        ? (fullName ? vpi_get_str(vpiFullName, actual) : vpi_get_str(vpiName, actual))
        : std::to_string(value.value.integer);
      //out += "\n";
    }
    else {
      out += "/*A*/";
      out += fullName ? vpi_get_str(vpiFullName, actual) :
        vpi_get_str(vpiName, actual);
    }
  }
  else {
    //out += "REF_OBJ Type: ";
    //out += std::to_string(((const uhdm_handle *)h)->type);
    fflush(stdout);
    out += "/*NA";
    out += std::to_string(fullName);
    if(!fullName) {
      if(vpiHandle xx = vpi_handle(vpiParent, h)) {
        out += " P:";
        out += vpi_get_str(vpiName, xx);
      }
    }
    out += "*/";
    out += fullName ? vpi_get_str(vpiFullName, h) :
      vpi_get_str(vpiName, h);
  }
  return out;
}


std::string visitLeaf(vpiHandle h) {
  std::string out = "";
  switch(((const uhdm_handle *)h)->type) {
    case UHDM::uhdmoperation : {
      out += " ( ";
      out += visitOperation(h);
      out += " ) ";

      //out += "ERROR_found_op";
      //TODO if expression is part of range, it has to be decompiled
      break;
    }
    case UHDM::uhdmconstant : {
      out += "/*CONST*/";
      out += "(";
      out += vpi_get_str(vpiDecompile, h);
      out += ")";
      //s_vpi_value value;
      //vpi_get_value(h, &value);
      //if (value.format) {
      //  out += std::to_string(value.value.integer);
      //} else out += "NVALUE";
      break;
    }
    case UHDM::uhdmparameter : {
      std::string d = vpi_get_str(vpiDecompile, h);
      out += "/*PARAM*/";
      out += d;
      break;
    }
    case UHDM::uhdmhier_path : 
      out += "/*STRUCT*/";
      out += visithier_path(h);
      break;
    case UHDM::uhdmref_obj : {
      //out += "RefObj within expr";
      out += visitref_obj(h);
      //int d = vpi_get(vpiDecompile, h);
      //
      //if(d) {
      //  out += "ERROR ref_obj (likely wrong): ";
      //  out += std::to_string(d);
      //}
      //else out += "ERROR Unresolved still";
      break;
    }
    case UHDM::uhdmbit_select :
      out += visitbit_sel(h, true);
      break;
    case UHDM::uhdmpart_select :
      out += visitpart_sel(h, true);
      break;
    case UHDM::uhdmindexed_part_select :
      out += visitindexed_part_sel(h);
      break;
    default: 
      out += "UNKNOWN_TYPE(";
      out += std::to_string(((const uhdm_handle *)h)->type);
      out += ")";
      break;
  }
  return out;
}

std::string visitbit_sel(vpiHandle h, bool fullName = true) {
  std::string out = "";
  //out += "in bit select  ";
  vpiHandle par = vpi_handle(vpiParent, h);
  if(!par) out += "\t\t\tERROR Couldn't find parent of bit_sel!\n";
  out += visitref_obj(par, fullName);
  out += "[";
  vpiHandle ind = vpi_handle(vpiIndex, h);
  //visitLeaf checks for type: uhdmconstant / uhdmparameter
  if(ind) out += visitLeaf(ind);
  else out += "ERROR Index not resolved!";
  out += "]";
  return out;
}

std::string visitindexed_part_sel(vpiHandle h) {
  std::string out = "";
  //std::string out = "in bit select  ";
  vpiHandle par = vpi_handle(vpiParent, h);
  if(!par) out += "\t\t\tERROR Couldn't find parent of bit_sel!\n";
  out += visitref_obj(par, true);
  //TODO vpiBaseExpr, vpiWidthExpr
  //out += "[";
  //vpiHandle ind = vpi_handle(vpiIndex, h);
  ////visitLeaf checks for type: uhdmconstant / uhdmparameter
  //if(ind) out += visitLeaf(ind);
  //else out += "ERROR Index not resolved!";
  //out += "]";
  return out;
}

std::string visitpart_sel(vpiHandle h, bool fullName= true) {
  std::string out = "";
  out += "";
  //out += "TYPE: ";
  //out += std::to_string(((uhdm_handle *)h)->type);
  vpiHandle par = vpi_handle(vpiParent, h);
  //if(!par) //D(out += "couldn't find parent of bit_sel!\n";)
  if(par) out += visitref_obj(par, fullName);
  else out += "\t\t\tParent not found";
  out += "[";
  vpiHandle lrh = vpi_handle(vpiLeftRange, h);
  //out += "  " +  std::to_string((((const uhdm_handle *)lrh)->type));
  if(lrh) out += visitLeaf(lrh);
  else out += "\t\t\tLeft range not found";
  out += ":";
  vpiHandle rrh = vpi_handle(vpiRightRange, h);
  //out += "  " +  std::to_string((((const uhdm_handle *)rrh)->type));
  if(rrh) out += visitLeaf(rrh);
  else out += "\t\t\tRight range not found";
  out += "]";
  vpi_release_handle(rrh);
  vpi_release_handle(lrh);
  return out;
}

//TODO bit_sel within hier_path pritning parent twice!!
std::string visithier_path(vpiHandle soph) {
  std::string out = "";
  //out += "\t\t\tSTRUCT found\n";
  vpiHandle it = vpi_iterate(vpiActual, soph);
  if(it) {
    bool first = true;
    while(vpiHandle itx = vpi_scan(it)) {
      //out += "\t\t\tFound one of the ref_objs: ";
      //out += std::to_string(((const uhdm_handle *)itx)->type);
      //out += "\n";
      fflush(stdout);
      if(first) {
        out += "/*FIRST*/";
        out += visitCond(itx, true);
      }
      else {
        out += "./*SECOND*/";
        out += visitCond(itx, false);
      }
      fflush(stdout);
      //out += "\n";
      first = false;
    }
  } else out += "ERROR couldn't iterate through 2 ref objs!\n";
  return out;
}

//TODO add a switch to select between print variables or expression
std::string visitOperation(vpiHandle h) {
  std::string out = "";
  const int type = vpi_get(vpiOpType, h);
  std::cout << "Operation type: " << std::to_string(type) << std::endl;
  std::string symbol = "";
  switch(type) {
    // some of the operations cannot appear
    // within control expressions; and so are not included here
    case 3  : symbol += " !  "; break;
    case 4  : symbol += " ~  "; break;
    case 5  : symbol += " &  "; break;
    case 7  : symbol += " |  "; break;
    case 11 : symbol += " -  "; break;
    case 14 : symbol += " == "; break;
    case 15 : symbol += " != "; break;
    case 16 : symbol += " ==="; break;
    case 17 : symbol += " !=="; break;
    case 18 : symbol += " >  "; break;
    case 19 : symbol += " >= "; break;
    case 20 : symbol += " <  "; break;
    case 21 : symbol += " <= "; break;
    case 22 : symbol += " << "; break;
    case 23 : symbol += " >> "; break;
    case 24 : symbol += " +  "; break;
    case 25 : symbol += " *  "; break;
    case 26 : symbol += " && "; break;
    case 27 : symbol += " || "; break;
    case 28 : symbol += " &  "; break;
    case 29 : symbol += " |  "; break;
    case 30 : symbol += " ^  "; break;
    case 32 : symbol += " :  "; break; 
    case 33 : symbol += " ,  "; break; //concat
    case 34 : symbol += "  { "; break;
    case 41 : symbol += " <<<"; break;
    case 42 : symbol += " >>>"; break;
    case 67 : symbol += " '( "; break;
    case 71 : symbol += " ,  "; break; //streaming left to right
    case 72 : symbol += " ,  "; break; //streaming right to left
    case 95 : symbol += " ,  "; break; 
    default : symbol += " UNKNOWN_OP(" + std::to_string(type) + ") " ; break;
  }

  vpiHandle op = vpi_iterate(vpiOperand, h);
  if(op) {
    int opCnt = 0;

    //priming
    if(type == 33)
      out += "{";
    else if(type == 71)
      out += "{>>{";
    else if(type == 72)
      out += "{<<{";
    else if(type == 3 || type == 4 || type == 5 || type == 7)
      out += symbol;

    while(vpiHandle oph = vpi_scan(op)) {
      std::cout << "Walking on opearands\n";
      if(opCnt == 0) {
        if(((const uhdm_handle *)oph)->type == UHDM::uhdmhier_path)
          out += visithier_path(oph);
        else if(((const uhdm_handle *)oph)->type == UHDM::uhdmoperation) {
          out += "(";
          out += visitOperation(oph);
          out += ")";
        }
        else {
          out += visitLeaf(oph);
        }
        opCnt++;
      } else {
        if(opCnt == 1) 
          if(type == 32)
            out += " ? ";
          else if(type == 95)
            out += " inside { ";
          else out += symbol;
        else    
          out += symbol;
        opCnt++;
        if(((const uhdm_handle *)oph)->type == UHDM::uhdmhier_path)
          out += visithier_path(oph);
        else if(((const uhdm_handle *)oph)->type == UHDM::uhdmoperation) {
          out += "(";
          out += visitOperation(oph);
          out += ")";
        }
        else
          out += visitLeaf(oph);
      }

      vpi_release_handle(oph);
    }
    vpi_release_handle(op);
    //wrap-up
    //TODO if x in {y,...}
    if(type == 33 ||
       type == 34 || 
       type == 95 || 
       type == 71 || 
       type == 72)
      out += " }";
    else if(type == 67) 
      out += " )";
  } else {
    std::cerr << "Couldn't iterate on operands! Iterator type: " << 
      std::to_string(((const uhdm_handle *)op)->type) << std::endl;
  }
  return out;
}

std::string visitCond(vpiHandle h, bool fullName = true) {
  std::cout << "Condition type: " 
    << std::to_string(((const uhdm_handle *)h)->type) << std::endl;
  std::string out = "";
  switch(((const uhdm_handle *)h)->type) {
    case UHDM::uhdmpart_select :
    case UHDM::uhdmindexed_part_select :
    case UHDM::uhdmbit_select :
    case UHDM::uhdmref_obj :
    case UHDM::uhdmexpr :
      std::cout << "Condition has leaf elements\n";
      out += visitLeaf(h);
      break;
    case UHDM::uhdmhier_path :
      std::cout << "Condition has struct element\n";
      out += visithier_path(h);
      break;
    case UHDM::uhdmoperation :
      std::cout << "Condition has an operation\n";
      out += visitOperation(h);
      break;
    case UHDM::uhdmconstant :
    case UHDM::uhdmparameter : 
      std::cout << "Condition has a constant/param; ignoring\n";
    default: 
      std::cout << "UNKNOWN element in condition\n";
      out += std::to_string(((const uhdm_handle *)h)->type);
      break;
  }
  return out;
}

void visitIfElse(vpiHandle h) {
  std::string out = "";
  std::cout << "Found IfElse/If\n";
  if(vpiHandle c = vpi_handle(vpiCondition, h)) {
    std::cout << "Found condition\n";
    out += visitCond(c);
    vpi_release_handle(c);
  } else std::cerr << "No condition found\n";
  std::cout << out << std::endl;
  ifs.push_back(out);

  if(vpiHandle s = vpi_handle(vpiStmt, h)) {
    std::cout << "Found statements\n";
    visitBlocks(s);
    vpi_release_handle(s);
  } else std::cout << "Statements not found\n";
  return;
}

void visitCase(vpiHandle h) {
  std::string out = "";
  if(vpiHandle c = vpi_handle(vpiCondition, h)) {
    std::cout << "Found condition\n";
    out += visitCond(c);
    vpi_release_handle(c);
  } else std::cout << "No condition found!\n";
  std::cout << out << std::endl;
  cases.push_back(out);

  std::cout << "Parsing case item; type: " << 
    std::to_string(((const uhdm_handle *)h)->type) << std::endl;
  vpiHandle newh = vpi_iterate(vpiCaseItem, h);
  if(newh) {
    while(vpiHandle sh = vpi_scan(newh)) {
      std::cout << "Found case item; type: " << 
        std::to_string(((const uhdm_handle *)sh)->type) << std::endl;
      visitBlocks(sh);
      vpi_release_handle(sh);
    }
    vpi_release_handle(newh);
  } else std::cout << "Statements not found\n";
  return;
}

void visitAssignment(vpiHandle h) {
  std::cout << "Walking assignment\n";
  if(vpiHandle rhs = vpi_handle(vpiRhs, h)) {
    if(((uhdm_handle *)rhs)->type == UHDM::uhdmoperation) {
      std::cout << "Operation found in RHS\n";
      const int n = vpi_get(vpiOpType, rhs);
      if (n == 32) {
        visitTernary(rhs);
      }
      else {
        if(vpiHandle operands = vpi_iterate(vpiOperand, rhs)) {
          while(vpiHandle operand = vpi_scan(operands)) {
            std::cout << "Operand type: ";
            std::cout << std::to_string(((uhdm_handle *)operand)->type);
            if(((uhdm_handle *)operand)->type == UHDM::uhdmoperation) {
              std::cout << "\nOperand is operation; checking if ternary\n";
              findTernaryInOperation(operand);
              vpi_release_handle(operand);
            }
          }
          vpi_release_handle(operands);
        }
      }
    } else
      std::cout << "Not an operation on the RHS; ignoring\n";
    vpi_release_handle(rhs);
  } else
    std::cerr << "Assignment without RHS handle\n";
  return;
}

void visitBlocks(vpiHandle h) {
  std::string out = "";
  std::cout << "Block type: " 
    << std::to_string(((const uhdm_handle *)h)->type) << std::endl;
  switch(((const uhdm_handle *)h)->type) {
    case UHDM::uhdmcase_items : 
    case UHDM::uhdmbegin : {
      vpiHandle i;
      i = vpi_iterate(vpiStmt,h);
      while (vpiHandle s = vpi_scan(i) ) {
        visitBlocks(s);
        vpi_release_handle(s);
      }
      vpi_release_handle(i);
      break;
    }
    //TODO what is this for?
    case UHDM::uhdmstmt :
      if(((const uhdm_handle *)h)->type == UHDM::uhdmevent_control) 
        visitBlocks(h);
      break;
    case UHDM::uhdmcase_stmt :
      std::cout << "Case statement found\n";
      visitCase(h);
      break;
    case UHDM::uhdmif_stmt :
    case UHDM::uhdmelse_stmt : 
    case UHDM::uhdmif_else :
      //TODO ignore if/ifelse blocks with systemfuncs only
      std::cout << "If/IfElse statement found\n";
      visitIfElse(h);
      if(vpiHandle el = vpi_handle(vpiElseStmt, h)) {
        std::cout << "Else statement found\n";
        visitIfElse(el);
      } else std::cout << "Didn't find else statement\n";
      break;
    case UHDM::uhdmalways : {
      vpiHandle newh = vpi_handle(vpiStmt, h);
      visitBlocks(newh);
      vpi_release_handle(newh);
      break;
    }
    case UHDM::uhdmassignment : {
      std::cout << "Assignment found; checking for ternaries\n";
      visitAssignment(h);
      break;
    }
    default :
      if(vpiHandle newh = vpi_handle(vpiStmt, h)) {
        std::cout << "UNKNOWN type; but statement found inside\n";
        visitBlocks(newh);
      } else {
        std::cerr << "UNKNOWN type; skipping processing this node\n";
        //Accommodate all cases eventually
      }
      break;
  }
  return;
}

void findTernaryInOperation(vpiHandle h) {
  std::string out = "\nIn findTernary\n";
  if(((uhdm_handle *)h)->type == UHDM::uhdmoperation) { //safety check
    const int nk = vpi_get(vpiOpType, h);
    //std::cout << "Type " << std::to_string(nk) << "\n";
    if(nk == 32) {
      std::cout << "An operand is ternary\n";
      visitTernary(h);
    }

  }
  return;
}

void visitTernary(vpiHandle h) {
  std::list <std::string> current;
  std::cout << "Analysing ternary operation\n";
  vpiHandle i = vpi_iterate(vpiOperand, h);
  bool first = true;
  if(i) {
    while (vpiHandle op = vpi_scan(i)) {
      int t = ((const uhdm_handle *)op)->type;
      std::cout << "Operand type: " << t << std::endl;
      switch(((const uhdm_handle *)op)->type) {
        case UHDM::uhdmoperation :
          {
            std::cout << "Operation found in ternary\n";
            findTernaryInOperation(op);
            if(first) {
              std::string out = visitOperation(op);
              current.push_front(out);
              first = false;
            }
            break;
          }
        case UHDM::uhdmref_obj :
        case UHDM::uhdmpart_select :
        case UHDM::uhdmbit_select :
        case UHDM::uhdmconstant : 
        case UHDM::uhdmparameter :
        case UHDM::uhdmexpr : //TODO look into this again!
          if(first) {
            std::cout << "Leaf found in ternary\n";
            std::string out = visitLeaf(op);
            current.push_front(out);
            first = false;
          }
          break;
        case UHDM::uhdmhier_path :
          if(first) {
            std::cout << "Struct found in ternary\n";
            std::string out = visithier_path(op);
            current.push_front(out);
            first = false;
          }
          break;
        default: 
          if(first) {
            std::cerr << "UNKNOWN type in ternary";
          }
          break;
      }
      vpi_release_handle(op);
    }
    vpi_release_handle(i);
  } else
    std::cout << "Couldn't iterate through statements!!\n";
  std::cout << "Saving ternaries...\n";
  print_list(current);
  ternaries.splice(ternaries.end(), current);
  return;
}

void visitVariables(vpiHandle i) {
  std::cout << "Walking variables\n"; 
  std::list <std::string> current;
  while (vpiHandle h = vpi_scan(i)) {
    std::string out = "";
    int t = vpi_get(vpiNetType, h);
    std::string type = t == 48 ? "Reg" : 
      "Wire(" + std::to_string(((const uhdm_handle *)h)->type) + ")";
    out += type + " -> " + vpi_get_str(vpiFullName, h);

    switch(((const uhdm_handle *)h)->type) {
      case UHDM::uhdmstruct_var :
        //std::cout << "Finding width of struct\n";
        //vpiHandle mem = vpi_iterate(vpiMember, h);
        //out_f += std::to_string(vpi_get(vpiSize, mem));
        //if(mem) 
        //  while (vpiHandle memit = vpi_scan(mem)) {
        //    out_f += "X";
        //    out_f += std::to_string(((const uhdm_handle *)memit)->type);
        //  }
        //else out_f += "No members found";
        ////out_f += visithier_path(mem);
        break;
      case UHDM::uhdmenum_var :
        break;
      case UHDM::uhdmarray_var :
        break;
      default :
        break;
    }

    vpiHandle ranges;
    if((ranges = vpi_iterate(vpiRange, h))) {
      while (vpiHandle range = vpi_scan(ranges) ) {
        out += "[";
        vpiHandle lh = vpi_handle(vpiLeftRange, range);
        out += visitLeaf(lh);
        out += ":";
        vpiHandle rh = vpi_handle(vpiRightRange, range);
        out += visitLeaf(rh);
        out += "]";
        vpi_release_handle(range);
        vpi_release_handle(lh);
        vpi_release_handle(rh);
      }
    } else out += ""; //[0:0]
    current.push_front(out);
    vpi_release_handle(ranges);
    vpi_release_handle(h);
  }
  std::cout << "No more nets\n";
  //print_list(current);
  nets.splice(nets.end(), current);
  vpi_release_handle(i);
  return;
}

void visitTopModules(vpiHandle ti) {
  std::cout << "Exercising iterator\n";
  while(vpiHandle th = vpi_scan(ti)) {
    std::cout << "Top module handle obtained\n";
    if (vpi_get(vpiType, th) != vpiModule) {
      std::cerr << "Not a module\n";
      return;
    }

    std::cout << "Proceeding\n";
    //lambda for module visit
    std::function<void(vpiHandle, std::string)> visit =
      [&visit](vpiHandle mh, std::string depth) {

        std::string out_f;
        std::string defName;
        std::string objectName;
        if (const char* s = vpi_get_str(vpiDefName, mh)) {
          defName = s;
        }
        if (const char* s = vpi_get_str(vpiName, mh)) {
          if (!defName.empty()) {
            defName += " ";
          }
          objectName = std::string("(") + s + std::string(")");
        }
        std::string file = "";
        if (const char* s = vpi_get_str(vpiFile, mh))
          file = s;
        std::cout << "Walking module: " + defName + objectName + "\n";// +
        ", file:" + file +
        ", line:" + std::to_string(vpi_get(vpiLineNo, mh)) + "\n";

        // Variables
        if(vpiHandle vi = vpi_iterate(vpiVariables, mh)) {
          std::cout << "Found variables\n"; 
          visitVariables(vi);
        } else std::cout << "No variables found in current module\n";

        // Nets
        if(vpiHandle ni = vpi_iterate(vpiNet, mh)) {
          std::cout << "Found nets\n";
          visitVariables(ni);
        } else std::cout << "No nets found in current module\n";

        // ContAssigns:
        //TODO struct assignment "bp_be/src/v/bp_be_calculator/bp_be_pipe_aux.sv" Ln 338
        //, unicore.dut.c[0].dut.cache.decode_v_r.decode_v_r.data_size_op[0][0]
        //TODO some assigns (not wire assignments) are missed -- fix it
        vpiHandle ci = vpi_iterate(vpiContAssign, mh);
        if(ci) {
          std::cout << "Found continuous assign statements\n";
            while (vpiHandle ch = vpi_scan(ci)) {
              std::cout << "Info -> " +
                std::string(vpi_get_str(vpiFile, ch)) +
                ", line:" + std::to_string(vpi_get(vpiLineNo, ch)) + "\n";
              visitAssignment(ch);
              vpi_release_handle(ch);
            }
          vpi_release_handle(ci);
        } else std::cout << "No continuous assign statements found in current module\n";

        //ProcessStmts:
        vpiHandle ai = vpi_iterate(vpiProcess, mh);
        if(ai) {
          std::cout << "Found always block\n";
            while(vpiHandle ah = vpi_scan(ai)) {
              out_f += "Info -> \n";// +
              //std::string(vpi_get_str(vpiFile, abh)) +
              //", line:" + std::to_string(vpi_get(vpiLineNo, abh)) + "\n";
              //function
              visitBlocks(ah);
              vpi_release_handle(ah);
            }
          vpi_release_handle(ai);
        } else std::cout << "No always blocks in current module\n";

        // Recursive tree traversal
        if (vpi_get(vpiType, mh) == vpiModule ||
            vpi_get(vpiType, mh) == vpiGenScope) {
          vpiHandle i = vpi_iterate(vpiModule, mh);
          while (vpiHandle h = vpi_scan(i)) {
            std::cout << "Iterating next module\n";
            depth = depth + "  ";
            visit(h, depth);
            vpi_release_handle(h);
          }
          vpi_release_handle(i);
        } else
          std::cout << "No children modules found\n";

        if (vpi_get(vpiType, mh) == vpiModule ||
            vpi_get(vpiType, mh) == vpiGenScope) {
          vpiHandle i = vpi_iterate(vpiGenScopeArray, mh);
          while (vpiHandle h = vpi_scan(i)) {
            std::cout << "Iterating genScopeArray\n";
            depth = depth + "  ";
            visit(h, depth);
            vpi_release_handle(h);
          }
          vpi_release_handle(i);
        } else
         std::cout << "No children genScopeArray found\n";

        if (vpi_get(vpiType, mh) == vpiGenScopeArray) {
          vpiHandle i = vpi_iterate(vpiGenScope, mh);
          while (vpiHandle h = vpi_scan(i)) {
            std::cout << "Iterating genScope\n";
            visit(h, depth);
            vpi_release_handle(h);
          }
          vpi_release_handle(i);
        } else
          std::cout << "No children genScope found\n";
        return;
      };
    visit(th, "");
    vpi_release_handle(th);
  }
  vpi_release_handle(ti);
  return;
}

int main(int argc, const char** argv) {
  // Read command line, compile a design, use -parse argument
  unsigned int code = 0;
  SURELOG::SymbolTable* symbolTable = new SURELOG::SymbolTable();
  SURELOG::ErrorContainer* errors = new SURELOG::ErrorContainer(symbolTable);

  SURELOG::CommandLineParser* clp =
    new SURELOG::CommandLineParser(errors, symbolTable, false, false);
  clp->noPython();
  clp->setParse(true);
  clp->setwritePpOutput(true);
  clp->setCompile(true);
  clp->setElaborate(true);  // Request Surelog instance tree Elaboration
  // clp->setElabUhdm(true);  // Request UHDM Uniquification/Elaboration

  bool success = clp->parseCommandLine(argc, argv);
  errors->printMessages(clp->muteStdout());
  vpiHandle the_design = 0;
  SURELOG::scompiler* compiler = nullptr;
  if (success && (!clp->help())) {
    compiler = SURELOG::start_compiler(clp);
    the_design = SURELOG::get_uhdm_design(compiler);
    auto stats = errors->getErrorStats();
    code = (!success) | stats.nbFatal | stats.nbSyntax | stats.nbError;
  }

  std::string out = "";

  std::cout << "UHDM Elaboration...\n";
  UHDM::Serializer serializer;
  UHDM::ElaboratorListener* listener =
    new UHDM::ElaboratorListener(&serializer, false);
  listen_designs({the_design}, listener);
  std::cout << "Listener in place\n";

  // Browse the UHDM Data Model using the IEEE VPI API.
  // See third_party/Verilog_Object_Model.pdf

  // Either use the
  // - C IEEE API, (See third_party/UHDM/tests/test_helper.h)
  // - or C++ UHDM API (See third_party/UHDM/headers/*.h)
  // - Listener design pattern (See third_party/UHDM/tests/test_listener.cpp)
  // - Walker design pattern (See third_party/UHDM/src/vpi_visitor.cpp)

  if (the_design) {
    UHDM::design* udesign = nullptr;
    if (vpi_get(vpiType, the_design) == vpiDesign) {
      // C++ top handle from which the entire design can be traversed using the
      // C++ API
      udesign = UhdmDesignFromVpiHandle(the_design);
      std::cout << "Design name (C++): " + udesign->VpiName() + "\n";
    }
    // Example demonstrating the classic VPI API traversal of the folded model
    // of the design Flat non-elaborated module/interface/packages/classes list
    // contains ports/nets/statements (No ranges or sizes here, see elaborated
    // section below)
    std::cout <<
      "Design name (VPI): " + std::string(vpi_get_str(vpiName, the_design)) +
      "\n";
    // Flat Module list:
    std::cout << "Module List:\n";
    //      topmodule -- instance scope
    //        allmodules -- assign (ternares), always (if, case, ternaries)

    vpiHandle ti = vpi_iterate(UHDM::uhdmtopModules, the_design);
    if(ti) {
      std::cout << "Walking uhdmtopModules\n";
      // The walk
      visitTopModules(ti);
    } else std::cout << "No uhdmtopModules found!";
  }
  //std::list <std::string> :: iterator it;
  //for(it = save.begin(); it != save.end(); ++it)
  //    std::cout << "\tLIST " << *it;
  //std::cout << '\n';

  std::cout << "\n\n\n*** Parsing Complete!!! ***\n\n\n";

  //TODO save to file
  std::cout << out << std::endl;

  // Do not delete these objects until you are done with UHDM
  SURELOG::shutdown_compiler(compiler);
  delete clp;
  delete symbolTable;
  delete errors;
  return code;
}
