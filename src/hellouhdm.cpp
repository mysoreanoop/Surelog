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
std::string visitoperation(vpiHandle);
std::string visitternary(vpiHandle, bool);
std::string visitstmt(vpiHandle);
std::string visitcond(vpiHandle, bool);

// global variables
bool saveVariables = false;
std::list <std::string> save;


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
      //int val = vpi_get(vpiDecompile, h);
      out += " ( ";
      out += visitoperation(h);
      out += " ) ";
      //out += std::to_string(val);

      //out += "ERROR_found_op";
      //out += vpi_get_str(vpiDecompile, h);
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
      //} else out += "Format not set!";
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
        out += visitcond(itx, true);
      }
      else {
        out += "./*SECOND*/";
        out += visitcond(itx, false);
      }
      fflush(stdout);
      //out += "\n";
      first = false;
    }
  } else out += "ERROR couldn't iterate through 2 ref objs!\n";
  return out;
}

//TODO add a switch to select between print variables or expression
std::string visitoperation(vpiHandle h) {
  std::string out, debug = "";
  //std::string out += "\t\t\tFiguring out type of operation\n";
  //out += "\t\t\tOpType: ";
  const int type = vpi_get(vpiOpType, h);
  std::string symbol = "";
  switch(type) {
    // some of the operations have no chance of occurring 
    // within control expressions; and so are not included here
    case 3  : symbol += " !  "; break;
    case 4  : symbol += " ~  "; break;
    case 5  : symbol += " &  "; break; //reduce
    case 7  : symbol += " |  "; break; //reduce
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
    case 34 : symbol += "  { "; break; //replication
    case 41 : symbol += " <<<"; break; //replication
    case 42 : symbol += " >>>"; break; //replication
    case 67 : symbol += " '( "; break; //variable replication
    case 71 : symbol += " ,  "; break; //streaming left to right
    case 72 : symbol += " ,  "; break; //streaming right to left
    case 95 : symbol += " ,  "; break; 
    default : symbol += " UNKNOWN_OP(" + std::to_string(type) + ") " ; break;
  }

  //out += std::to_string(type) + "(" + symbol + ")";
  //out += "\n";
  vpiHandle sopi = vpi_iterate(vpiOperand, h);
  if(sopi) {
    //out += "\t\t\tSome operation with:\n"; 
    int opCnt = 0;
    //prologue
    if(type == 33)
      out += "{";
    else if(type == 71)
      out += "{>>{";
    else if(type == 72)
      out += "{<<{";
    else if(type == 3 || type == 4 || type == 5 || type == 7)
      out += symbol;

    while(vpiHandle soph = vpi_scan(sopi)) {
      if(opCnt == 0) {
        if(((const uhdm_handle *)soph)->type == UHDM::uhdmhier_path)
          out += visithier_path(soph);
        else if(((const uhdm_handle *)soph)->type == UHDM::uhdmoperation) {
          out += "(";
          out += visitoperation(soph);
          out += ")";
        }
        else {
          out += visitLeaf(soph);
        }
        opCnt++;
      }

      //switch(((const uhdm_handle *)soph)->type) {
      //  case UHDM::uhdmbit_select :
      //    out += visitbit_sel(soph);
      //    break;
      //  case UHDM::uhdmpart_select : 
      //    out += visitpart_sel(soph);
      //    break;
      //  case UHDM::uhdmoperation :
      //    out += " ( ";
      //    out += visitoperation(soph);
      //    out += " ) ";
      //    break;
      //  case UHDM::uhdmref_obj :
      //    out += visitref_obj(soph);
      //    break;
      //  case UHDM::uhdmconstant :
      //    out += "/* decompiled */";
      //    out += std::to_string(vpi_get(vpiDecompile, soph));
      //    break;
      //  case UHDM::uhdmhier_path : 
      //    out += visithier_path(soph);
      //    break;
      //  default : {
      //    out += "UNKNOWN_OPERAND";//visitref_obj(aa);
      //    out += std::to_string(((const uhdm_handle *)soph)->type);
      //    break;
      //  }
      //}
        //first symbol anomalies
      else {
        if(opCnt == 1) 
          if(type == 32)
            out += " ? ";
          else if(type == 95)
            out += " inside { ";
          else out += symbol;
        else    
          out += symbol;
        opCnt++;
        if(((const uhdm_handle *)soph)->type == UHDM::uhdmhier_path)
          out += visithier_path(soph);
        else if(((const uhdm_handle *)soph)->type == UHDM::uhdmoperation) {
          out += "(";
          out += visitoperation(soph);
          out += ")";
        }
        else
          out += visitLeaf(soph);

      }

      vpi_release_handle(soph);
    }
    vpi_release_handle(sopi);
    //epilogue
    if(type == 33 ||
       type == 34 || 
       type == 95 || 
       type == 71 || 
       type == 72)
      out += " }";
    else if(type == 67) 
      out += " )";
    //out += "\n";
  } 
  else {
    out += "ERROR: Couldn't iterate on operands!! Op iterator type: ";
    out += std::to_string(((const uhdm_handle *)sopi)->type);
    out += "\n";
  }
  return out;
}

std::string visitcond(vpiHandle h, bool fullName = true) {
  //std::string out = "\t\t\tEvaluating condition...\n";
  std::string out = "";
  switch(((const uhdm_handle *)h)->type) {
    case UHDM::uhdmpart_select :
      //out += "\t\t\tpart_sel: \n";
      out += visitpart_sel(h, fullName);
      break;
    case UHDM::uhdmindexed_part_select :
      //out += "\t\t\tindexed_part_sel: \n";
      out += visitindexed_part_sel(h);
      break;
    case UHDM::uhdmbit_select :
      out += "/*B*/";
      out += visitbit_sel(h, fullName);
      break;
    case UHDM::uhdmref_obj :
      out += "/*R*/";
      out += visitref_obj(h, fullName);
      break;
    case UHDM::uhdmexpr :
      out += " FOUND_EXPR ";
      out += visitLeaf(h);
      break;
    case UHDM::uhdmhier_path :
      out += "/*H*/";
      out += visithier_path(h);
      break;
    case UHDM::uhdmoperation :
      //out += "\t\t\tOPERATION: ";
      out += visitoperation(h);
      out += "\n";
      break;
    case UHDM::uhdmconstant :  //XXX verify that we can skip this; we're priting constants if they appear as part of expressions with ref_objs
    case UHDM::uhdmparameter : 
			out += visitLeaf(h);
    default: 
      out += "\t\t\tUnanticipated object in control expr: ";
      out += std::to_string(((const uhdm_handle *)h)->type);
      break;
  }
  return out;
}

std::string visitIfElse(vpiHandle h) {
  std::string out = "";
  //out += "Found IfElse/If: ";
  //out += vpi_get_str(vpiFullName, h);
  if(vpiHandle iff = vpi_handle(vpiCondition, h)) {
    out += "\t\t\tFound condition\n";
    out += visitcond(iff);
    out += "\n";
  } //else out += "No condition found!!\n";
  //XXX useless!!
  //if(vpiHandle els = vpi_handle(vpiElseStmt, h)) {
  //  out += "\t\t\tIn Else stamtement type: ";
  //  out += std::to_string(((const uhdm_handle *)els)->type);
  //  //out += visitIfElse(els);
  //  out += "\t\t\tIn Else stamtement exiting\n";
  //}
  //      else out += "\nNo else statement!\n";
  if(vpiHandle newh = vpi_handle(vpiStmt, h)) {
    out += "STARTING STATEMENT PARSING!\n";
    out += visitstmt(newh);
  } out += "\t\t\tstatements not found\n";
  ////XXX Change back newh to h
  //out += "\t\t\tFound internal statement: ";
  //out += UHDM::UhdmName(((const uhdm_handle *)newh)->type);
  //out += "\n";
  //if(vpiHandle rhs = vpi_handle(vpiRhs, newh)) {
  //  //Expression
  //  const int n = vpi_get(vpiOpType, rhs);
  //  if (n == 32) {
  //    out += "Found a ternary within if/else statement body: \n";
  //    out += visitternary(rhs);
  //    //res += "\n";
  //  } else out += "Found not a ternary!\n";
  //  vpi_release_handle(rhs);
  //} else {
  //  out += "\t\t\tLooking to see if this is begin, and can skip this\n";
  //  if (((const uhdm_handle *)newh)->type == UHDM::uhdmbegin) {
  //    out += "\t\t\tFound begin, skipping ahead..\n";
  //    //vpiHandle kh = vpi_handle(vpiStmt, newh);
  //    //if(vpiHandle krhs = vpi_handle(vpiRhs, kh)) {
  //    //  const int k = vpi_get(vpiOpType, krhs);
  //    //  if(k == 32) {
  //    //    out += "Found a ternary within if/else (after begin) statement body: \n";
  //    //    out += visitternary(rhs);
  //    //    //res += "\n";
  //    //  } else out += "Found not a ternary after begin!\n";
  //    //}
  //  } 
  //}
  //}

return out;
}

std::string visitCase(vpiHandle h) {
  std::string out = "";
    if(vpiHandle iff = vpi_handle(vpiCondition, h)) {
    out += "\t\t\tFound condition\n";
    out += visitcond(iff);
    out += "\n";
  } //else out += "No condition found!!\n";
  out += "\t\t\tParsing case items... type: ";
  out += std::to_string(((const uhdm_handle *)h)->type);
  vpiHandle newh = vpi_iterate(vpiCaseItem, h);
  if(newh) {
    while(vpiHandle sh = vpi_scan(newh)) {
      out += "\t\t\tFound case item: \n";
      out += std::to_string(((const uhdm_handle *)sh)->type);
      //if(vpiHandle s = vpi_handle(vpiStmt, sh))
      //  out += "\t\t\tFound statement within case item\n";
      //else
      //  out += "\t\t\tFound nothing within case item\n";
      out += visitstmt(sh);
    }
  }else out += "\t\t\tDidn't find internal statements\n";
  return out;

}

std::string visitstmt(vpiHandle h) {
  std::string out = "\t\tAM: In visitstmt for ";
  out += std::to_string(((const uhdm_handle *)h)->type);
  out += "\n";
  switch(((const uhdm_handle *)h)->type) {
    case UHDM::uhdmcase_items : 
    case UHDM::uhdmbegin : {
      //      out += "Found begin statement\n";
      //vpiHandle s = vpi_handle(vpiStmt, h);
      //if(s) out += visitstmt(s);
      vpiHandle itr;
      itr = vpi_iterate(vpiStmt,h);
      while (vpiHandle obj = vpi_scan(itr) ) {
        //        out += "Found statements inside Begin\n";
        out += visitstmt(obj);
        //        out += std::to_string(((const uhdm_handle *)h)->type);
      }
      vpi_release_handle(itr);
      break;
    }
    case UHDM::uhdmstmt :
      //vpiHandle stmt = vpi_handle(vpiStmt, h);
      //      out += "\tAM: Statement found:";
      //      out += std::to_string(((const uhdm_handle *)h)->type);
      //      out += "\n";
      if(((const uhdm_handle *)h)->type == UHDM::uhdmevent_control) 
        //  || ((const uhdm_handle *)stmt)->type == UHDM::uhdmbegin) {
        //        out += "\tAM: Found event_control\n";
        out += visitstmt(h);
      break;
    case UHDM::uhdmcase_stmt : {
      out += "\t\tCase found\n";
      out += visitCase(h);
      break;
    }
      //TODO check for "$display/$fwrite only" IF bodies 
    case UHDM::uhdmelse_stmt : 
    case UHDM::uhdmif_stmt :
    case UHDM::uhdmif_else : { 
      out += "\t\tIf/ElseIf found\n";
      out += visitIfElse(h);
      if(vpiHandle el = vpi_handle(vpiElseStmt, h)) {
        out += "\t\tFound else counterpart: \n";
        out += visitIfElse(el);
      } else out += "\t\tDidn't find else statement!\n";
      break;
    }
    case UHDM::uhdmalways : {
      //out += "\t\tAM: Always found: \n";
      vpiHandle newh = vpi_handle(vpiStmt, h);
      out += visitstmt(newh);
      vpi_release_handle(newh);
      break;
    }
    case UHDM::uhdmassignment : {
      out += "\t\tFound assignment!!\n";
      vpiHandle newh = vpi_handle(vpiRhs, h);
      if(newh) {
        int type = ((const uhdm_handle *)newh)->type;
        out += "\t\tFound Rhs of type:  ";
        out += std::to_string(type);
        out += "\n";
        if(type == UHDM::uhdmoperation) {
          const int n = vpi_get(vpiOpType, newh);
          out += "\t\tOpType:  ";
          out += std::to_string(n);
          out += "\n";
          if(n == 32) {
            out += "\t\tTernary Operation in Rhs\n";
            out += visitternary(newh, false);
            out += "\n";
          } else if(n== 33) {
            out += "\t\tconcat op in Rhs\n";
            vpiHandle it = vpi_iterate(vpiOperand, newh);
            if(it) {
              while (vpiHandle aa = vpi_scan(it)) {
                if(((const uhdm_handle *)aa)->type == UHDM::uhdmoperation) {
                  const int k = vpi_get(vpiOpType, aa);
                  if(k == 32) {
                    out += "\t\tTernary in concat\n";
                    out += visitternary(aa, false);
                    out += "\n";
                  }
                }
              }
            }
          }
        } 
      }
      break;
    }
    default :
      if(vpiHandle newh = vpi_handle(vpiStmt, h))
        out += visitstmt(newh);
      else {
        out += "\t\tFound something different in always block: ";
        out += std::to_string(((const uhdm_handle *)h)->type);
        out += "\n";
      }
      break;

  }
  return out;
}
std::string findTernaryInOperation(vpiHandle h) {
  std::string out = "\nIn findTernary\n";
  if(((uhdm_handle *)h)->type == UHDM::uhdmoperation) { //safety check
    const int nk = vpi_get(vpiOpType, h);
    out += "Type ";
    out += std::to_string(nk);
    out += "\n";
    if(nk == 32) {
      out += "revisiting ternary\n";
      out += visitternary(h, false);
      out += "\n";
    }
  }
  return out;
}

std::string visitternary(vpiHandle h, bool saveOperands = false) {
  std::string out = "";
  out += "\t\tAM: Ternary operator recognized:\n";
  vpiHandle opi = vpi_iterate(vpiOperand, h);
  bool first = true;
  if(opi) {
    while (vpiHandle aa = vpi_scan(opi)) {
      //out += "\t\tObject type: ";// (operation/part_select/constant)
      switch(((const uhdm_handle *)aa)->type) {
        case UHDM::uhdmoperation : //ternary and regular operations
          {
            out += findTernaryInOperation(aa);
            out += "\t\tOperation found\n";
            if(first) {
              out += visitoperation(aa);
              save.push_front(out);
              first = false;
            }
            break;
          }
        case UHDM::uhdmref_obj :
        case UHDM::uhdmpart_select :
        case UHDM::uhdmbit_select :
        case UHDM::uhdmconstant : 
        case UHDM::uhdmparameter :
        case UHDM::uhdmexpr :
          if(first) {
            out += "Expression found...\n";
            out += visitLeaf(aa);
            save.push_front(out);
            out += "\n";
            first = false;
            break;
          }
        case UHDM::uhdmhier_path :
          if(first) {
            out += visithier_path(aa);
            save.push_front(out);
            first = false;
          } else out += "\n";
          break;
        default: 
          if(first) {
            out += "UNKNOWN";
            save.push_front(out);
            out += std::to_string(((const uhdm_handle *)aa)->type);
          }
          out += "\n";
          break;
      }

      vpi_release_handle(aa);
    }
    vpi_release_handle(opi);
  }
  else out += "Couldn't iterate through statements!!\n";
  return out;
}


std::string visitModules(vpiHandle mi) {
  std::string out = "AM: Top module found\n";
  while(vpiHandle mh = vpi_scan(mi)) {
    if (vpi_get(vpiType, mh) != vpiModule) {
      out += "ERROR: this is not a module\n";
    }
    std::function<std::string(vpiHandle, std::string)> inst_visit =
      [&inst_visit](vpiHandle obj_h, std::string margin) {
        std::string res;
        std::string defName;
        std::string objectName;
        if (const char* s = vpi_get_str(vpiDefName, obj_h)) {
          defName = s;
        }
        if (const char* s = vpi_get_str(vpiName, obj_h)) {
          if (!defName.empty()) {
            defName += " ";
          }
          objectName = std::string("(") + s + std::string(")");
        }
        std::string f;
        if (const char* s = vpi_get_str(vpiFile, obj_h)) {
          f = s;
        }
        res += margin + "AM: In module: " + defName + objectName + "\n";// +
        //", file:" + f +
        //", line:" + std::to_string(vpi_get(vpiLineNo, obj_h)) + "\n";

        //Nets: TODO indent the prints properly!
        if(vpiHandle ni = vpi_iterate(vpiVariables, obj_h)) {
          res += "AM: Found variables!!!\n"; 
          vpiHandle nh;
          while ((nh = vpi_scan(ni)) != NULL) {
            res += "\tAM: In net -> ";
            const char *reg = (vpi_get(vpiNetType, nh) == 48 )  ? "Reg  " : "Wire ";
            res += reg;
            //res += "ScopeType(";
            //if(const char * s = vpi_get_str(vpiResolvedNetType, nh))
            //  res += s;
            //else res += "not found";
            //res += ") ";
            res += vpi_get_str(vpiFullName, nh);
            vpiHandle ri;
            //TODO to print widths properly
            //res += std::to_string(((const uhdm_handle *)nh)->type);
            //switch(((const uhdm_handle *)nh)->type) {
            //  case UHDM::uhdmstruct_var : {
            //    res += "Finding width of STRUCT:\n";
            //    vpiHandle mem = vpi_iterate(vpiMember, nh);
            //    res += std::to_string(vpi_get(vpiSize, mem));
            //    if(mem) 
            //      while (vpiHandle memit = vpi_scan(mem)) {
            //        res += "X";
            //        res += std::to_string(((const uhdm_handle *)memit)->type);
            //      }
            //    else res += "No members found";
            //    //res += visithier_path(mem);
            //    break;
            //  }
            //  case UHDM::uhdmenum_var :
            //    break;
            //  case UHDM::uhdmarray_var :
            //    break;
            //  default :
            //    break;
            //}
            if((ri = vpi_iterate(vpiRange, nh))) {
              while (vpiHandle rh = vpi_scan(ri) ) {
                //res += " Range ";
                res += " [";
                vpiHandle lrh = vpi_handle(vpiLeftRange, rh);
                res += visitLeaf(lrh);
                res += ":";
                vpiHandle rrh = vpi_handle(vpiRightRange, rh);
                res += visitLeaf(rrh);
                res += "]";
                vpi_release_handle(rh);
                vpi_release_handle(lrh);
                vpi_release_handle(rrh);
              }
            } else res += " [0]";
            res += "\n";
            vpi_release_handle(nh);
          }
          res += "\tAM: No more nets found\n";
          vpi_release_handle(ni);
        }// else res += "AM: nets not found\n";
        if(vpiHandle ni = vpi_iterate(vpiNet, obj_h)) {
          res += "AM: Found nets!!!\n";
          vpiHandle nh;
          while ((nh = vpi_scan(ni)) != NULL) {
            res += "\tAM: In net -> ";
            const char *reg = (vpi_get(vpiNetType, nh) == 48 )  ? "Reg  " : "Wire ";
            res += reg;
            //res += "ScopeType(";
            //if(const char * s = vpi_get_str(vpiResolvedNetType, nh))
            //  res += s;
            //else res += "not found";
            //res += ") ";
            res += vpi_get_str(vpiFullName, nh);
            vpiHandle ri;
            //TODO to print widths properly -- some are 0 even though they have + value
            //res += std::to_string(((const uhdm_handle *)nh)->type);
            //switch(((const uhdm_handle *)nh)->type) {
            //  case UHDM::uhdmstruct_var : {
            //    res += "Finding width of STRUCT:\n";
            //    vpiHandle mem = vpi_iterate(vpiMember, nh);
            //    if(mem) 
            //      while (vpiHandle memit = vpi_scan(mem)) {
            //        res += "X";
            //        res += std::to_string(((const uhdm_handle *)memit)->type);
            //      }
            //    else res += "No members found";
            //    //res += visithier_path(mem);
            //    break;
            //  }
            //  case UHDM::uhdmenum_var :
            //    break;
            //  case UHDM::uhdmarray_var :
            //    break;
            //  default :
            //    break;
            //}
            if((ri = vpi_iterate(vpiRange, nh))) {
              while (vpiHandle rh = vpi_scan(ri) ) {
                //res += " Range ";
                res += " [";
                vpiHandle lrh = vpi_handle(vpiLeftRange, rh);
                res += visitLeaf(lrh);
                res += ":";
                vpiHandle rrh = vpi_handle(vpiRightRange, rh);
                res += visitLeaf(rrh);
                res += "]";
                vpi_release_handle(rh);
                vpi_release_handle(lrh);
                vpi_release_handle(rrh);
              }
            } else res += " [0]";
            res += "\n";
            vpi_release_handle(nh);
          }
          res += "\tAM: No more nets found\n";
          vpi_release_handle(ni);
        }// else res += "AM: nets not found\n";

        //ContAssigns:
        //TODO struct assignment "bp_be/src/v/bp_be_calculator/bp_be_pipe_aux.sv" Ln 338
        //, unicore.dut.c[0].dut.cache.decode_v_r.decode_v_r.data_size_op[0][0]
        //TODO some assigns (not wire assignments) are missed -- fix it
        vpiHandle ai = vpi_iterate(vpiContAssign, obj_h);
        if(ai) {
          D(res += "\tAM: Found assign!\n";)
            while (vpiHandle ah = vpi_scan(ai)) {
              res += "\tAM: In assign -> " +
                std::string(vpi_get_str(vpiFile, ah)) +
                ", line:" + std::to_string(vpi_get(vpiLineNo, ah)) + "\n";
              if(vpiHandle rhs = vpi_handle(vpiRhs, ah)) { 
              //RHS
                if(((uhdm_handle *)rhs)->type == UHDM::uhdmoperation) {
                  //Operation
                  const int n = vpi_get(vpiOpType, rhs);
                  if (n == 32) {
                    res += visitternary(rhs);
                    res += "\n";
                  }
                  else {
                    if(vpiHandle operands = vpi_iterate(vpiOperand, rhs))
                      while(vpiHandle operand = vpi_scan(operands)) {
                        res += "type of operation: ";
                        res += std::to_string(((uhdm_handle *)operand)->type);
                        res += "\n";
                        //vpiHandle op = vpi_handle(vpiOpType, rhs);
                        res += findTernaryInOperation(rhs);
                      } 
                  }
                  std::cout << "Previous ternary accumulated " << save.size() << "elements\n";
                } else {
                  res += "Not an operation on the RHS of assign statement\n";
                }
                vpi_release_handle(rhs);
              }
              vpi_release_handle(ah);
            }
          vpi_release_handle(ai);
        } else  res += "\tAM: assigns not found\n";

        //ProcessStmts:
        vpiHandle abi = vpi_iterate(vpiProcess, obj_h);
        if(abi) {
          D(res += "\tAM: Found always block\n";)
            while(vpiHandle abh = vpi_scan(abi)) {
              res += "\tAM: In always -> \n";// +
              //std::string(vpi_get_str(vpiFile, abh)) +
              //", line:" + std::to_string(vpi_get(vpiLineNo, abh)) + "\n";
              //function
              res += visitstmt(abh);
              vpi_release_handle(abh);
            }
          vpi_release_handle(abi);
        }// else  res += "AM: ALWAYS BLOCKS not found\n";

        // Recursive tree traversal
        //margin = "\\__" + margin;
        if (vpi_get(vpiType, obj_h) == vpiModule ||
            vpi_get(vpiType, obj_h) == vpiGenScope) {
          //res += "SUBITERATION!!\n";
          vpiHandle subItr = vpi_iterate(vpiModule, obj_h);
          while (vpiHandle sub_h = vpi_scan(subItr)) {
            res += inst_visit(sub_h, margin);
            vpi_release_handle(sub_h);
          }
          vpi_release_handle(subItr);
        }
        if (vpi_get(vpiType, obj_h) == vpiModule ||
            vpi_get(vpiType, obj_h) == vpiGenScope) {
          vpiHandle subItr = vpi_iterate(vpiGenScopeArray, obj_h);
          while (vpiHandle sub_h = vpi_scan(subItr)) {
            //res += "SUBITERATION!!\n";
            res += inst_visit(sub_h, margin);
            vpi_release_handle(sub_h);
          }
          vpi_release_handle(subItr);
        }
        if (vpi_get(vpiType, obj_h) == vpiGenScopeArray) {
          vpiHandle subItr = vpi_iterate(vpiGenScope, obj_h);
          while (vpiHandle sub_h = vpi_scan(subItr)) {
            //res += "SUBITERATION!!\n";
            res += inst_visit(sub_h, margin);
            vpi_release_handle(sub_h);
          }
          vpi_release_handle(subItr);
        }
        return res;
      };
    out += inst_visit(mh, "");
    vpi_release_handle(mh);
  }
  return out;
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

  std::string out;
  std::string prints = "";

  // If UHDM is not already elaborated/uniquified (uhdm db was saved by a
  // different process pre-elaboration), then ~optionally~ elaborate it:
  std::cout << "UHDM Elaboration...\n";
  UHDM::Serializer serializer;
  UHDM::ElaboratorListener* listener =
    new UHDM::ElaboratorListener(&serializer, false); //XXX 
  listen_designs({the_design}, listener);

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
      out += "Design name (C++): " + udesign->VpiName() + "\n";
    }
    // Example demonstrating the classic VPI API traversal of the folded model
    // of the design Flat non-elaborated module/interface/packages/classes list
    // contains ports/nets/statements (No ranges or sizes here, see elaborated
    // section below)
    out +=
      "Design name (VPI): " + std::string(vpi_get_str(vpiName, the_design)) +
      "\n";
    // Flat Module list:
    out += "Module List:\n";
    //      topmodule -- instance scope
    //        allmodules -- assign (ternares), always (if, case, ternaries)



    prints = out;
    vpiHandle mkk = vpi_iterate(UHDM::uhdmtopModules, the_design);
    if(mkk) {
      D(out += "AM: Some topmodule iterator found\n";)
        //Nets:
        out += visitModules(mkk);
    } else out += "No modules found!";

    //vpiHandle mi =  vpi_iterate(UHDM::uhdmallModules, the_design);
    //if(mi) {
    //  prints += "AM: Some allmodule iterator found\n";
    //  while(vpiHandle obj_h = vpi_scan(mi)) {
    //    prints += "AM: In ";
    //    //prints += vpi_get_str(vpiFullName, obj_h);
    //    //prints += "\n";
    //    //ContAssigns:
    //    //vpiHandle ai = vpi_iterate(vpiContAssign, obj_h);
    //    //if(ai) {
    //    //  while (vpiHandle ah = vpi_scan(ai)) {
    //    //    prints += "\tAM: In assign -> " +
    //    //        std::string(vpi_get_str(vpiFile, ah)) +
    //    //        ", line:" + std::to_string(vpi_get(vpiLineNo, ah)) + "\n";
    //    //      //RHS
    //    //      if(vpiHandle rhs = vpi_handle(vpiRhs, ah)) { 
    //    //        //Expression
    //    //        const int n = vpi_get(vpiOpType, rhs);
    //    //        if (n == 32) {
    //    //          prints += visitternary(rhs);
    //    //        }
    //    //        vpi_release_handle(rhs);
    //    //      }
    //    //    vpi_release_handle(ah);
    //    //  }
    //    //  vpi_release_handle(ai);
    //    //} else prints += "\tAM: No assigns found!";

    //    //ProcessStmts:
    //    //vpiHandle abi = vpi_iterate(vpiProcess, obj_h);
    //    //if(abi) {
    //    //  while(vpiHandle abh = vpi_scan(abi)) {
    //    //    //prints += "\tAM: In always -> ";
    //    //    //std::string(vpi_get_str(vpiFile, abh)) +
    //    //    //prints +=  ", line:" + std::to_string(vpi_get(vpiLineNo, abh)) + "\n";
    //    //    //function
    //    //    prints += visitstmt(abh);
    //    //    vpi_release_handle(abh);
    //    //  }
    //    //  
    //    //  vpi_release_handle(abi);
    //    //} else prints += "No pricess iterations found\n";
    //  }
    //} else prints += "No modules found!";

  }
  //std::list <std::string> :: iterator it;
  //for(it = save.begin(); it != save.end(); ++it)
  //    std::cout << "\tLIST " << *it;
  //std::cout << '\n';

  out += "\n\n\n*** Parsing Complete!!! ***\n\n\n";
  std::cout << out << std::endl;
  //std::cout << prints << std::endl;

  // Do not delete these objects until you are done with UHDM
  SURELOG::shutdown_compiler(compiler);
  delete clp;
  delete symbolTable;
  delete errors;
  return code;
}
