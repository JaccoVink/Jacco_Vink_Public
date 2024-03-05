/*****************************************************************************/
//
// FILE        :  jvtools_messages.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION : 
//
//
// COPYRIGHT   : Jacco Vink,  2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "jvtools_messages.h"

using JVMessages::FunctionID;
//
//
//

#define MESSAGETAG "\033[32;mMESSAGE -\033[0m"
#define WARNINGTAG "\033[35;1;mWARNING -\033[0m"
#define ERRORTAG "\033[31;1;mERROR -\033[0m"

namespace JVMessages{

  FILE *errorPtr = stdout;
  FILE *messagePtr = stdout;
  const int fnameLength = 21;
  char functionID[fnameLength], oldID[fnameLength]=" ";
  int nerrors  = 0;
  int nwarnings = 0;

  ChatLevel chat = CHATTY;
  FunctionID functionLabel;


  FunctionID::FunctionID()
  {
    functionName = "main";
    deeper = 0;
  }//constructor
  FunctionID::~FunctionID()
  {
    if( deeper != 0){
      delete deeper;
    }
  }//constructor
  const char* FunctionID::functionLabel()
  {
    FunctionID *ptr=this;
    while(  ptr->deeper != 0 )
      ptr = ptr->deeper;
    return ptr->functionName.c_str();
  };
  Status FunctionID::newLabel( const char labelName[] )
  {
    if( deeper == 0 ){
      deeper = new FunctionID;
      deeper->functionName = labelName;
    }
    else
      deeper->newLabel( labelName );
    return SUCCESS;
  }//newLabel
  Status FunctionID::deleteLabel( )
  {

#if 1
    //    puts("delete");
    //    printf("new label :\n");
    FunctionID *ptr=this;
    while( ptr != 0 ){
      //      printf("\t%s\n", ptr->functionName.c_str());
      ptr=ptr->deeper;
    }
    //    puts("**********");
#endif
    if( deeper != 0 ){
      FunctionID *ptr = deeper, *prev=this;
      while(  ptr->deeper != 0 ){
	prev = ptr;
	ptr = ptr->deeper;
      }
      //      printf("deleting %s\n", ptr->functionName.c_str());
      delete ptr;
      prev->deeper=0;

    }
    return SUCCESS;
  }



  //
  //
  //
  void setFunctionID( const char name[] ){
    //    if( !(chat == SUPPRESS_ALL_MESSAGES) ){
      strncpy(oldID, functionID, fnameLength-1);
      strncpy(functionID, name, fnameLength-1);
      functionLabel.newLabel( functionID);
  }
  //};

  //
  //
  //
  int numberOfErrors( void )
  {
    return nerrors;
  }//numberOfErrors()

  //
  //
  //
  Status error(const char message[] )
  {
    if( chat != SUPPRESS_ALL_MESSAGES )
      fprintf(errorPtr, "\t%s %20s: %s!\n", ERRORTAG, 
	      functionLabel.functionLabel(), message);
    nerrors++;
    strncpy(functionID, oldID, fnameLength-1);
    functionLabel.deleteLabel();

    return FAILURE;
  };
  Status error(const char message[], long status )
  {
    if( chat != SUPPRESS_ALL_MESSAGES )
      fprintf(errorPtr, "\t%s %20s: %s! %li\n", ERRORTAG, 
	      functionLabel.functionLabel(), message, status);

    nerrors++;
    strncpy(functionID, oldID, fnameLength-1);
    functionLabel.deleteLabel();

    return FAILURE;
  };
  Status error(const char message[], double value)
  {
    if( chat != SUPPRESS_ALL_MESSAGES ){
      if( fabs(value) > 1.0e-3 ){
	fprintf(errorPtr, "\t%s %20s: %s %f!\n", ERRORTAG, 
		functionLabel.functionLabel(), message, value);
      }else{
	fprintf(errorPtr, "\t%s %20s: %s %.4e!\n", ERRORTAG, 
		functionLabel.functionLabel(), message, value);
      }
    }
    nerrors++;
    strncpy(functionID, oldID, fnameLength-1);
    functionLabel.deleteLabel();

    return FAILURE;
  };

  Status error(const char message[], const char message2[] )
  {
    if( chat != SUPPRESS_ALL_MESSAGES )
      fprintf(errorPtr, "\t%s %20s: %s  %s\n", ERRORTAG, 
	      functionLabel.functionLabel(), message, message2);

    nerrors++;
    strncpy(functionID, oldID, fnameLength-1);
    functionLabel.deleteLabel();
    return FAILURE;
  };
  Status error(const char message[], int status ){
    return error(message,(long)status);};
  Status error(const char message[], float value){
    return error(message,(double)value);};

  //
  // Warnings
  //
  Status warning(const char message[] )
  {
    if( chat == CHATTY || chat == NO_MESSAGES )
      fprintf(errorPtr, "\t%s %20s: %s\n", WARNINGTAG, 
	      functionLabel.functionLabel(), message);
    nwarnings++;
    //    strncpy(functionID, oldID, fnameLength-1);
    return FAILURE;
  };
  Status warning(const char message[], const char messagestr[] )
  {
    if( chat == CHATTY || chat == NO_MESSAGES )
      fprintf(errorPtr, "\t%s %20s: %s %s\n", WARNINGTAG, 
	      functionLabel.functionLabel(), message, messagestr);
    nwarnings++;
    //    strncpy(functionID, oldID, fnameLength-1);
    return FAILURE;
  };
  Status warning(const char message[], long int status)
  {
    if( chat == CHATTY || chat == NO_MESSAGES )
      fprintf(errorPtr, "\t%s %20s: %s %li\n", WARNINGTAG, 
	      functionLabel.functionLabel(), message, status);
    nwarnings++;
    //    strncpy(functionID, oldID, fnameLength-1);
    return FAILURE;
  };
  Status warning(const char message[], double value)
  {
    if( chat == CHATTY || chat == NO_MESSAGES ){
      if( value!= 0.0 && (fabs(value) < 1.0e-3 || fabs(value) > 1.0e3) ){
	fprintf(errorPtr, "\t%s %20s: %s %.12e\n", WARNINGTAG, 
		functionLabel.functionLabel(), message, value);
      }else{
	fprintf(errorPtr, "\t%s %20s: %s %.6f\n", WARNINGTAG, 
		functionLabel.functionLabel(), message, value);
      }//else
    }//if
    nwarnings++;
    //    strncpy(functionID, oldID, fnameLength-1);
    return FAILURE;
  };

  //
  // Displays a message
  //
  Status message(const char message[] )
  {
    if( chat == CHATTY)
      fprintf(messagePtr, "\t%s %20s: %s\n", MESSAGETAG, 
	      functionLabel.functionLabel(), message);
    return SUCCESS;
  };
  Status message(const char message[], double value )
  {
    if( chat == CHATTY){
      if(  value!= 0.0 && (fabs(value) < 1.0e-2 || fabs(value) > 1.0e4 )){
	fprintf(messagePtr, "\t%s %20s: %s %e\n", MESSAGETAG, 
		functionLabel.functionLabel(), message, value);
      }else{
	fprintf(messagePtr, "\t%s %20s: %s %f\n", MESSAGETAG, 
		functionLabel.functionLabel(), message, value);
      }//else
    }//if
    return SUCCESS;
  };
  Status message(const char message[], float value )
  {
    if( chat == CHATTY){
      if( fabs(value) < 1.0e-2 || fabs(value) > 1.0e4 ){
	fprintf(messagePtr, "\t%s %20s: %s %.12e\n", MESSAGETAG, 
		functionLabel.functionLabel(), message, value);
      }else{
	fprintf(messagePtr, "\t%s %20s: %s %f\n", MESSAGETAG, 
		functionLabel.functionLabel(), message, value);
      }//else
    }//if
    return SUCCESS;
  };
  Status message(const char message[], int value)
  {
    if( chat == CHATTY)
      fprintf(messagePtr, "\t%s %20s: %s %i\n", MESSAGETAG, 
	      functionLabel.functionLabel(), message, value);
    return SUCCESS;
  };
  Status message(const char message[], long value)
  {
    if( chat == CHATTY)
      fprintf(messagePtr, "\t%s %20s: %s %li\n", MESSAGETAG, 
	      functionLabel.functionLabel(), message, value);
    return SUCCESS;
  };
  Status message(const char message[], unsigned long value)
  {
    if( chat == CHATTY)
      fprintf(messagePtr, "\t%s %20s: %s %lu\n", MESSAGETAG, 
	      functionLabel.functionLabel(), message, value);
    return SUCCESS;
  };

  Status message(const char message[], const char messageStr[] )
  {
    if( chat == CHATTY)
      fprintf(messagePtr, "\t%s %20s: %s %s\n", MESSAGETAG, 
	      functionLabel.functionLabel(), message, messageStr);
    return SUCCESS;
  };
  Status message(const char messageStr1[], const string &messageStr2 )
  {
    return message(messageStr1, messageStr2.c_str());
  };

  Status success( void )
  {
#if 0
    if( chat != SUPPRESS_ALL_MESSAGES){
      strncpy(functionID, oldID, fnameLength-1);
      functionLabel.deleteLabel();
    }
#endif
    strncpy(functionID, oldID, fnameLength-1);
    functionLabel.deleteLabel();
    
    return SUCCESS;
  };

  Status setChatLevel( ChatLevel newLevel ){
    //    message("chat level changed");
    chat = newLevel;
    return SUCCESS;
  };
  ChatLevel getChatLevel( void ){
    return chat;
  }
};
