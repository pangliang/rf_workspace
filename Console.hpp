//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹?
//?                                                                        ?
//?Module: Header file for console interface                               ?
//?                                                                        ?
//?Description: Interface declarations for internals plugin                ?
//?                                                                        ?
//?This source code module, and all information, data, and algorithms      ?
//?associated with it, are part of isiMotor Technology (tm).               ?
//?                PROPRIETARY                                             ?
//?Copyright (c) 1996-2006 Image Space Incorporated.  All rights reserved. ?
//?                                                                        ?
//?Change history:                                                         ?
//?  jmm.2004.11.29: created                                               ?
//?                                                                        ?
//ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ?

#ifndef CONSOLE_HPP
#define CONSOLE_HPP

//***************************************************************************//
// Included to give access to windows types and methods
#include <windows.h>
//***************************************************************************//

//***************************************************************************//
// Maximum size of read buffer
#define MAX_CONSOLE_INPUT_BUFFER_SIZE 16384 // 16K
//***************************************************************************//

//***************************************************************************//
class Console
{
private:
  //*************************************************************************//
  HANDLE mhStdError;
  HANDLE mhStdInput;
  HANDLE mhStdOutput;
  //*************************************************************************//
public:
  //*************************************************************************//
  //-------------------------------------------------------------------------//
  // Constuctor/Destructor
  Console();
  ~Console();
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Sets the title of the Console window
  void SetTitle(char* cpConsoleTitle);
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // The below Method will read from the console window and will set cpInputbuffer 
  // to the data read from the console.  The amount of data read can be set
  // by passing it to the method via cusBufferSize.  cusBufferSize must be less 
  // than MAX_CONSOLE_INPUT_BUFFER_SIZE
  long Read(char* cpInputbuffer, const unsigned short cusBufferSize = MAX_CONSOLE_INPUT_BUFFER_SIZE);
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // The below method will write the data in cpOutputBuffer to console window.
  // If usBufferSize is not specified then this method will assum the data in 
  // cpOutputBuffer is null terminated. If bWriteToStdError is true then this
  // mthod will write to standard error otherwise it will write to standard output
  long Write(const void* cpOutputBuffer, unsigned short usBufferSize = 0, const bool bWriteToStdError = false);
  //-------------------------------------------------------------------------//
  //*************************************************************************//
};
//***************************************************************************//

#endif CONSOLE_HPP