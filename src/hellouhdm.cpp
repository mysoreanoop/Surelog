#include <functional>
#include <iostream>

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
/*
   TODO statments within case -- not checking for ternaries
   -- try and look for case items and then statemetns?

 */
std::string visitref_obj(vpiHandle h) {
  std::string result = "\t\t\t";
  vpiHandle actual = vpi_handle(vpiActual, h);
  if(actual) {
    //result += std::to_string(((const uhdm_handle *)actual)->type);
    if(((const uhdm_handle *)actual)->type == UHDM::uhdmenum_const) {
      //TODO not working!!!!
      result += std::to_string(vpi_get(vpiDecompile, actual));
    }
    else if(((const uhdm_handle *)actual)->type == UHDM::uhdmparameter) {
      result += "PARAM: ";
      result += std::to_string(vpi_get(vpiDecompile, actual));
      result += "(";
      result += vpi_get_str(vpiFullName, actual);
      result += ")";
      result += "\n";
    }
    else result += vpi_get_str(vpiFullName, actual);
  }
  else {
    result += vpi_get_str(vpiFullName, h);
  }
  return result;
}

std::string visitexpr(vpiHandle h) {
  std::string result = " OLAF: ";
  switch(((const uhdm_handle *)h)->type) {
    case UHDM::uhdmoperation : {
      int val = vpi_get(vpiDecompile, h);
      result += std::to_string(val);
      result += "ERROR found operation within expression!";
      //result += vpi_get_str(vpiDecompile, h);
      //TODO if expression is part of range -- tehn it has to be evaluated as expression/operation
      //TODO if parameter is part of range -- tehn it has to be evaluated as expression/operation
      break;
    }
    case UHDM::uhdmconstant : {
      result += "Found constant within expression: ";
      s_vpi_value value;
      vpi_get_value(h, &value);
      //TODO somehow reference visit_value in UHDM::??
      if (value.format) {
        result += std::to_string(value.value.integer);
      } else result += "Format not set!";
      break;
    }
    case UHDM::uhdmparameter : {
      int d = vpi_get(vpiDecompile, h);
      result += "Parameter decompiled: ";
      result += std::to_string(d);
      break;
    }
    case UHDM::uhdmref_obj : {
      result += "RefObj within expr";
      result += visitref_obj(h);
      //int d = vpi_get(vpiDecompile, h);
      //
      //if(d) {
      //  result += "ERROR ref_obj (likely wrong): ";
      //  result += std::to_string(d);
      //}
      //else result += "ERROR Unresolved still";
      break;
    }
    default: 
      result += "ERROR found type: ";
      result += std::to_string(((const uhdm_handle *)h)->type);
      result += ")";
      break;
  }
  return result;
}

std::string visitbit_sel(vpiHandle h) {
  std::string result = "";
  vpiHandle par = vpi_handle(vpiParent, h);
  if(!par) result += "\t\t\tERROR Couldn't find parent of bit_sel!\n";
  result += visitref_obj(par);
  result += "[";
  vpiHandle ind = vpi_handle(vpiIndex, h);
  //visitexpr checks for type: uhdmconstant / uhdmparameter
  if(ind) result += visitexpr(ind);
  else result += "ERROR Index not resolved!";
  result += "]\n";
  return result;
}

std::string visitpart_sel(vpiHandle h) {
  std::string result = "";
  result += "";
  //result += "TYPE: ";
  //result += std::to_string(((uhdm_handle *)h)->type);
  vpiHandle par = vpi_handle(vpiParent, h);
  //if(!par) //D(result += "couldn't find parent of bit_sel!\n";)
  if(par) result += visitref_obj(par);
  else result += "\t\t\tParent not found";
  result += "[";
  vpiHandle lrh = vpi_handle(vpiLeftRange, h);
  //result += "  " +  std::to_string((((const uhdm_handle *)lrh)->type));
  if(lrh) result += visitexpr(lrh);
  else result += "\t\t\tLeft range not found";
  result += ":";
  vpiHandle rrh = vpi_handle(vpiRightRange, h);
  //result += "  " +  std::to_string((((const uhdm_handle *)rrh)->type));
  if(rrh) result += visitexpr(rrh);
  else result += "\t\t\tLeft range not found";
  result += "]\n";
  vpi_release_handle(rrh);
  vpi_release_handle(lrh);
  return result;
}

//TODO verify
std::string visitoperation(vpiHandle aa) {
  //Can be comparison ops; LHS/RHS possibly being arithmetic/logical op
  std::string result = "\t\t\tFiguring out type of operation\n";
  result += "\t\t\tOpType: ";
  const int type = vpi_get(vpiOpType, aa);
  result += std::to_string(type);
  result += "\n";
  vpiHandle sopi = vpi_iterate(vpiOperand, aa);
  if(sopi) {
    result += "\t\t\tSome operation with:\n"; //just print it
    while(vpiHandle soph = vpi_scan(sopi)) {
      switch(((const uhdm_handle *)soph)->type) {
        case UHDM::uhdmbit_select :
          result += visitbit_sel(soph);
          break;
        case UHDM::uhdmpart_select : //TODO 
          result += visitpart_sel(soph);
          break;
        case UHDM::uhdmoperation :
          result += visitoperation(soph);
          break;
        case UHDM::uhdmref_obj :
          result += visitref_obj(soph);
          break;
        case UHDM::uhdmconstant :
          result += "\t\t\tConstant ignored";
          break;
        case UHDM::uhdmhier_path :
          result += "\t\t\tStruct found; can't resolve";
          break;
        default : {
          result += "\t\t\tNot a recognized type of operand: ";//visitref_obj(aa);
          result += std::to_string(((const uhdm_handle *)soph)->type);
          result += "\n";
          break;
        }
      }
      result += "\t\t\tand ..\n ";
      vpi_release_handle(soph);
    }
    vpi_release_handle(sopi);
    result += "\n";
  } 
  else {
    result += "uhdmoperation of type: ";
    result += std::to_string(((const uhdm_handle *)sopi)->type);
    result += "\n";
  }
  return result;
}

std::string visitCond(vpiHandle h) {
  std::string result = "\t\t\tEvaluating condition...\n";
  switch(((const uhdm_handle *)h)->type) {
    case UHDM::uhdmpart_select :
      result += "\t\t\tpart_sel: \n";
      result += visitpart_sel(h);
      result += "\n";
      break;
    case UHDM::uhdmref_obj :
      result += "\t\t\tref_obj: \n";
      result += visitref_obj(h);
      result += "\n";
      break;
    case UHDM::uhdmexpr :
      result += "\t\t\texpr: \n";
      result += visitexpr(h);
      result += "\n";
      break;
    case UHDM::uhdmhier_path :
      result += "\t\t\thier_paths: \n";
      //result += visitexpr(h);
      result += "\n";
      break;
    case UHDM::uhdmoperation :
      result += "\t\t\tOPERATION: \n";
      result += visitoperation(h);
      break;
    case UHDM::uhdmconstant : //not interesting
    case UHDM::uhdmparameter : //not interesting
      //FIXME ignoring ternary possibility in control expressions
    default: 
      result += "\t\t\tUnanticipated object in control expr.: ";
      result += std::to_string(((const uhdm_handle *)h)->type);
      result += "\n";
      break;
  }
  return result;
}

std::string visitTernary(vpiHandle h);
std::string visitStmt(vpiHandle h);
std::string visitIfElse(vpiHandle h) {
  std::string result = "";
  //result += "Found IfElse/If: ";
  //result += vpi_get_str(vpiFullName, h);
  if(vpiHandle iff = vpi_handle(vpiCondition, h)) {
    result += "\t\t\tFound condition\n";
    result += visitCond(iff);
  } //else result += "No condition found!!\n";
  if(vpiHandle els = vpi_handle(vpiElseStmt, h)) {
    //        result += "Else stamtement type: ";
    //        result += std::to_string(((const uhdm_handle *)els)->type);
    result += visitIfElse(els);
  }
  //      else result += "\nNo else statement!\n";
  if(vpiHandle newh = vpi_handle(vpiStmt, h)) {
    result += "STARTING STATEMENT PARSING!\n";
    result += visitStmt(newh);
    ////XXX Change back newh to h
    //result += "\t\t\tFound internal statement: ";
    //result += UHDM::UhdmName(((const uhdm_handle *)newh)->type);
    //result += "\n";
    //if(vpiHandle rhs = vpi_handle(vpiRhs, newh)) {
    //  //Expression
    //  const int n = vpi_get(vpiOpType, rhs);
    //  if (n == 32) {
    //    result += "Found a ternary within if/else statement body: \n";
    //    result += visitTernary(rhs);
    //    //res += "\n";
    //  } else result += "Found not a ternary!\n";
    //  vpi_release_handle(rhs);
    //} else {
    //  result += "\t\t\tLooking to see if this is begin, and can skip this\n";
    //  if (((const uhdm_handle *)newh)->type == UHDM::uhdmbegin) {
    //    result += "\t\t\tFound begin, skipping ahead..\n";
    //    //vpiHandle kh = vpi_handle(vpiStmt, newh);
    //    //if(vpiHandle krhs = vpi_handle(vpiRhs, kh)) {
    //    //  const int k = vpi_get(vpiOpType, krhs);
    //    //  if(k == 32) {
    //    //    result += "Found a ternary within if/else (after begin) statement body: \n";
    //    //    result += visitTernary(rhs);
    //    //    //res += "\n";
    //    //  } else result += "Found not a ternary after begin!\n";
    //    //}
    //  } 
    //}
  }
  else result += "\t\t\tDidn't find internal statements\n";
  return result;
}


std::string visitStmt(vpiHandle h) {
  std::string result = "\t\tAM: In visitStmt for ";
  result += std::to_string(((const uhdm_handle *)h)->type);
  result += "\n";
  switch(((const uhdm_handle *)h)->type) {
    case UHDM::uhdmbegin : {
      //      result += "Found begin statement\n";
      //vpiHandle s = vpi_handle(vpiStmt, h);
      //if(s) result += visitStmt(s);
      vpiHandle itr;
      itr = vpi_iterate(vpiStmt,h);
      while (vpiHandle obj = vpi_scan(itr) ) {
        //        result += "Found statements inside Begin\n";
        result += visitStmt(obj);
        //        result += std::to_string(((const uhdm_handle *)h)->type);
      }
      vpi_release_handle(itr);
      break;
    }
    case UHDM::uhdmstmt : {
      //vpiHandle stmt = vpi_handle(vpiStmt, h);
      //      result += "\tAM: Statement found:";
      //      result += std::to_string(((const uhdm_handle *)h)->type);
      //      result += "\n";
      if(((const uhdm_handle *)h)->type == UHDM::uhdmevent_control) 
        //  || ((const uhdm_handle *)stmt)->type == UHDM::uhdmbegin) {
        //        result += "\tAM: Found event_control\n";
        result += visitStmt(h);
      break;
    }
      case UHDM::uhdmcase_stmt : {
        result += "\t\tCase found\n";
        result += visitIfElse(h); //TODO have its own condition for checking case item body
        break;
      }
      case UHDM::uhdmelse_stmt :
      case UHDM::uhdmif_stmt :
      case UHDM::uhdmif_else : { 
        result += "\t\tIf/ElseIf found\n";
        result += visitIfElse(h);
        break;
      }
      case UHDM::uhdmalways : {
        //result += "\t\tAM: Always found: \n";
        vpiHandle newh = vpi_handle(vpiStmt, h);
        result += visitStmt(newh);
        vpi_release_handle(newh);
        break;
      }
      case UHDM::uhdmassignment : {
        result += "\t\tFound assignment!!\n";
        vpiHandle newh = vpi_handle(vpiRhs, h);
        if(newh) {
          int type = ((const uhdm_handle *)newh)->type;
          result += "\t\tFound Rhs of type:  ";
          result += std::to_string(type);
          result += "\n";
          if(type == UHDM::uhdmoperation) {
            const int n = vpi_get(vpiOpType, newh);
            result += "\t\tOpType:  ";
            result += std::to_string(n);
            result += "\n";
            if(n == 32) {
              result += "\t\tTernary Operation in Rhs\n";
              result += visitTernary(newh);
            } else if(n== 33) {
              result += "\t\tconcat op in Rhs\n";
              vpiHandle it = vpi_iterate(vpiOperand, newh);
              if(it) {
                while (vpiHandle aa = vpi_scan(it)) {
                  if(((const uhdm_handle *)aa)->type == UHDM::uhdmoperation) {
                    //TODO without "first" variable, the below prints all operands of ternary!!
                    const int k = vpi_get(vpiOpType, aa);
                    if(k == 32) {
                      result += "\t\tTernary in concat\n";
                      result += visitTernary(aa);
                    }
                  }
                }
              }
            }
          } 
        }
        //TODO ternary operations!!!
        break;
      }
      default : {
        if(vpiHandle newh = vpi_handle(vpiStmt, h))
          result += visitStmt(newh);
        else {
          result += "\t\tFound something different in always block: ";
          result += std::to_string(((const uhdm_handle *)h)->type);
          result += "\n";
        }
        break;

      }
    }
    return result;
  }

  std::string visitTernary(vpiHandle h) {
    std::string result = "";
    result += "\t\tAM: Ternary operator recognized:\n";
    vpiHandle opi = vpi_iterate(vpiOperand, h);
    bool first = true;
    if(opi) {
      while (vpiHandle aa = vpi_scan(opi)) {
        result += "\t\tObject type: ";// (operation/part_select/constant)
        switch(((const uhdm_handle *)aa)->type) {
          case UHDM::uhdmoperation : //ternary and regular operations
            {
              const int n = vpi_get(vpiOpType, aa);
              if(n == 32) {
                result += "\t\tAnother Ternary Operation\n";
                result += visitTernary(aa); //TODO camel case
              }
              else
                result += "\t\tNot a ternary within ternary, but could be an expression\n";
              if(first) {
                result += "\t\tFirst operation, not a ternary\n";
                result += visitoperation(aa);
              } 
              first = false;
              break;
            }
          case UHDM::uhdmref_obj :
            if(first) {
              result += "\t\tRef_obj\n";
              result += visitref_obj(aa);
              result += "\n";
              first = false;
            } //else just break
            break;
          case UHDM::uhdmpart_select :
            //fetch the ref_obj and print
            if(first) {
              result += "\t\tPart_sel\n";
              result += visitpart_sel(aa);
              result += "\n";
              first = false;
            } //else just break
            break;
          case UHDM::uhdmbit_select :
            //fetch the ref_obj and print
            if(first) {
              result += "\t\tBit_sel\n";
              result += visitbit_sel(aa);
              result += "\n";
            }
            break;
          case UHDM::uhdmconstant : 
          case UHDM::uhdmparameter :
            result += "\t\tFound constant or parameter; ignored\n";
            first = false;
            break;
            //ignored either way
            //          result += "Constant\n";
            //const int c = vpi_get(vpiConstType, aa);
            //result += std::to_string(c);
            //s_vpi_value value;
            //vpi_get_value(aa, &value);
            //if (value.format) {
            //  std::string val = std::to_string(value.value.scalar); //TODO can be integer also
            //  result += val;
            //}
          case UHDM::uhdmhier_path :
            //MAJOR TODO!!! vpiHierPath isn't fully printed!!
            result += "\t\tFound hierarchical path (struct)\n";
            //result += vpi_get_str(vpiFullName, aa); 
            first = false;
            break;
          default: 
            result += "Unknown type in ternary: ";
            //TODO concatenations don't show as expressions somehow!
            result += std::to_string(((const uhdm_handle *)aa)->type);
            result += "\n";
            break;
        }

        //    result += "\n";
        vpi_release_handle(aa);
      }
      vpi_release_handle(opi);
    }
    else result += "Couldn't iterate through statements!!\n";
    return result;
  }

  std::string visitModulesForNets(vpiHandle mi) {
    std::string result = "AM: Top module found\n";
    std::string nets = "AM: Nets only:\n";
    while(vpiHandle mh = vpi_scan(mi)) {
      if (vpi_get(vpiType, mh) != vpiModule) {
        result += "ERROR: this is not a module\n";
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
          if(vpiHandle ni = vpi_iterate(vpiNet, obj_h)) {
            res += "AM: Found nets!!!\n";
            vpiHandle nh;
            while ((nh = vpi_scan(ni)) != NULL) {
              res += "\tAM: In net -> ";
              const char *reg = (vpi_get(vpiNetType, nh) == 48 )  ? "Reg  " : "Wire ";
              res += reg;
              res += "ScopeType(";
              if(const char * s = vpi_get_str(vpiResolvedNetType, nh))
                res += s;
              else res += "not found";
              res += ")\n\t\t\t";
              res += vpi_get_str(vpiFullName, nh);
              vpiHandle ri;
              if((ri = vpi_iterate(vpiRange, nh))) {
                while (vpiHandle rh = vpi_scan(ri) ) {
                  //res += " Range ";
                  res += "[";
                  vpiHandle lrh = vpi_handle(vpiLeftRange, rh);
                  res += visitexpr(lrh);
                  res += ":";
                  vpiHandle rrh = vpi_handle(vpiRightRange, rh);
                  res += visitexpr(rrh);
                  res += "]";
                  vpi_release_handle(rh);
                  vpi_release_handle(lrh);
                  vpi_release_handle(rrh);
                }
              } //else res += "\t\tAM: Bit\n";
              res += "\n";
              vpi_release_handle(nh);
            }
            res += "\tAM: No more nets found\n";
            vpi_release_handle(ni);
          }// else res += "AM: nets not found\n";

          //ContAssigns:
          vpiHandle ai = vpi_iterate(vpiContAssign, obj_h);
          if(ai) {
            D(res += "\tAM: Found assign!\n";)
              while (vpiHandle ah = vpi_scan(ai)) {
                res += "\tAM: In assign -> " +
                  std::string(vpi_get_str(vpiFile, ah)) +
                  ", line:" + std::to_string(vpi_get(vpiLineNo, ah)) + "\n";
                //RHS
                if(vpiHandle rhs = vpi_handle(vpiRhs, ah)) { 
                  //Expression
                  const int n = vpi_get(vpiOpType, rhs);
                  if (n == 32) {
                    res += visitTernary(rhs);
                    //res += "\n";
                  }
                  else if (n==33 || n==34) { //concat/multi-concat
                    res += "\tFound concats:\n";
                    //vpiHandle op = vpi_handle(vpiOpType, rhs);
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
                res += "\tAM: In always -> ";// +
                //  std::string(vpi_get_str(vpiFile, abh)) +
                //  ", line:" + std::to_string(vpi_get(vpiLineNo, abh)) + "\n";
                //function
                res += visitStmt(abh);
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
      result += inst_visit(mh, "");
      vpi_release_handle(mh);
    }
    return result;
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

    std::string result;
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
        result += "Design name (C++): " + udesign->VpiName() + "\n";
      }
      // Example demonstrating the classic VPI API traversal of the folded model
      // of the design Flat non-elaborated module/interface/packages/classes list
      // contains ports/nets/statements (No ranges or sizes here, see elaborated
      // section below)
      result +=
        "Design name (VPI): " + std::string(vpi_get_str(vpiName, the_design)) +
        "\n";
      // Flat Module list:
      result += "Module List:\n";
      //      topmodule -- instance scope
      //        allmodules -- assign (ternares), always (if, case, ternaries)



      prints = result;
      vpiHandle mkk = vpi_iterate(UHDM::uhdmtopModules, the_design);
      if(mkk) {
        D(result += "AM: Some topmodule iterator found\n";)
          //Nets:
          result += visitModulesForNets(mkk);
      } else result += "No modules found!";

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
      //    //          prints += visitTernary(rhs);
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
      //    //    prints += visitStmt(abh);
      //    //    vpi_release_handle(abh);
      //    //  }
      //    //  
      //    //  vpi_release_handle(abi);
      //    //} else prints += "No pricess iterations found\n";
      //  }
      //} else prints += "No modules found!";

    }
    result += "\n\n\n***DONE PARSING!!!***\n\n\n";
    std::cout << result << std::endl;
    //std::cout << prints << std::endl;

    // Do not delete these objects until you are done with UHDM
    SURELOG::shutdown_compiler(compiler);
    delete clp;
    delete symbolTable;
    delete errors;
    return code;
  }
