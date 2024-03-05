/*****************************************************************************/
//
// FILE        :  history.cc
// AUTHOR      :  Jacco Vink 
// DESCRIPTION :  Implementation of History class, which stores "history"
//                information.
//
//
// COPYRIGHT   : Jacco Vink,  2002
//
/*****************************************************************************/
//  MODIFICATIONS :                                                           
//
//
/*****************************************************************************/
#include "jvtools_history.h"
#include "jvtools_messages.h"

using jvtools::History;
using JVMessages::error;
using JVMessages::warning;
using JVMessages::message;
using JVMessages::setFunctionID;
using JVMessages::success;

//
//
// Constructor
History::History()
{
  //  strncpy(text,"",HIST_LEN);
  text = 0;
  next = 0;
}

//
// Destructor
//
History::~History()
{

  if( next != 0 ){
    delete next;
    next = 0;
  }

  if( text != 0 ){
    delete [] text;
    text = 0;
  }

}// destructor

//
// Creates a new History object or passes the text on to the next
// object in the pointer list
//
Status History::insertText(const char newtext[])
{
  setFunctionID("insertText[history]");

  unsigned int strl = strlen(newtext)+1;
  if( strl >= HIST_LEN-2 ){
    char *newtext1 = new char [strl];
    char *newtext2 = new char [strl];
    strncpy( newtext1, newtext, strl-1);

    //    printf("string too long!");
    int i, ibreak=HIST_LEN-6;
    for(i=ibreak-1; i > 0 && (newtext1[i] != ' ' && newtext1[i]!= '/'); i--);
    if( i == 0 ){
      strncpy(newtext2, "[...] ", strl-1);
      strncat(newtext2, newtext+ibreak, strl-1);
      newtext1[HIST_LEN-3] = '\0';
      i=ibreak;
    }
    else{
      //      printf("space on pos %i %i\n", i, (int)newtext[i]);
      strncpy(newtext2, "[...] ", strl-1);
      strncat(newtext2, newtext+i+1, strl-1);
      //      printf("replaced char %c %i\n", newtext1[i+1], newtext1[i+1]);
      //      printf("-%s- -%s- \n",newtext1,newtext2);
      if( strncasecmp(newtext1, newtext2, strlen(newtext1) ) == 0 ){
	 for(i=ibreak-1; 
	     i > 0 && (newtext1[i] != ' ' && newtext1[i]!= '.' && 
		       newtext1[i] != '_'); i--);
	 strncpy(newtext2, "[...] ", strl-1);
	 strncat(newtext2, newtext+i+1, strl-1);
	 if( strncasecmp(newtext1, newtext2, strlen(newtext1) ) == 0 )
	   i=ibreak;
	 strncpy(newtext2, "[...] ", strl-1);
	 strncat(newtext2, newtext+i+1, strl-1);
	 newtext1[i+1] = '\\';      
	 i++;
      }
      newtext1[i+1] = '\0';      
    }
#if 0
    message("Splitted the history line in 2, original length was ",
	    (long int)strl);
    message("original: ",newtext1); 
    message("1st half: ",newtext1); 
    message("2nd half: ",newtext2); 
#endif
    insertText(newtext1);
    insertText(newtext2);

    delete [] newtext1;
    delete [] newtext2;
    return success();
  }//if

  if( text == 0 && strl > 1){
    text = new char [strl+1];
    if( text == 0 )
      return error("string allocation error");
    strncpy(text, newtext, strl);
  }
  else{
    if( next == 0 ){
      next = new History;
      if( next == 0 )
	return error("object allocation error");
    }
    next->insertText( newtext);
  }
  return success();
}//insertText()




//
//
// Adds the history texts to fits file
//
Status History:: add2FitsFile(fitsfile *fptr) const
{
  int status = 0;

  if( text != 0 )
    fits_write_history(fptr, text, &status );


  if( next != 0 )
    if( next->add2FitsFile( fptr ) == FAILURE )
      return FAILURE;

  if( status != 0 )
    return FAILURE;
  else
    return SUCCESS;
}// addToFitsFile



//
//
// Prints the entire list
//
Status History::print( FILE *fp) const
{
  if( text != 0 )
    fprintf(fp, "HISTORY %s\n", text);

  if( next != 0 )
    next->print(fp);

  return SUCCESS;
}// print

