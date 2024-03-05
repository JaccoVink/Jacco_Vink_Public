#ifndef __JV_MESSAGES__H__
#define __JV_MESSAGES__H__

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
#include "common.h"
#include <string>


//
// JVMessages contains functions and gloabl variables for exception handling
// and displaying messages in a uniform way.
//
using std::string;


namespace JVMessages{

  typedef enum chatlevel{
    CHATTY = 0, NO_MESSAGES = 1, NO_WARNINGS = 2, SUPPRESS_ALL_MESSAGES = 3
  } ChatLevel;//

  class FunctionID{
  public:
    FunctionID();
    ~FunctionID();
    Status newLabel( const char labelName[]);
    Status deleteLabel( );
    const char* functionLabel();
  private:
    string functionName;
    FunctionID *deeper;
  };//FunctionID;

  void setFunctionID( const char name[] );
  
  Status error(const char message[] );
  Status error(const char message[], double value);
  Status error(const char message[], long status );
  Status error(const char message[], int status );
  Status error(const char message[], float value);

  Status error(const char message[], const char message2[] );
  /*
  Status warning( string message){
    warning( message.c_str());
    return SUCCESS;
  };
  */
  Status warning(const char message[] );
  Status warning(const char message[], long int status );
  inline  Status warning(const char message[], int status ){
    return warning(message, (long int) status);};

  Status warning(const char message[], double value );
  inline Status warning(const char message[], float value ){
    return warning(message, (double) value);};

  Status warning(const char message[], const char messagestr[] );
  Status message(const char message[] );

  Status message(const char message[], int value );
  Status message(const char message[], long value );
  Status message(const char message[], unsigned long value );
  Status message(const char message[], double value );
  Status message(const char message[], float value );
  Status message(const char message[], const char messageStr[] );

  Status message(const char messageStr1[], const string &messageStr2 );
  //    { return message(messageStr1, messageStr2.c_str());};

  Status setChatLevel( ChatLevel newLevel=CHATTY );
  ChatLevel getChatLevel( void );
  int numberOfErrors( void );
  Status success( void );
};



#endif
