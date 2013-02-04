//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹?
//?                                                                        ?
//?Module: Source file for console                                         ?
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

//***************************************************************************//
#include "Console.hpp"
#include <assert.h>
//***************************************************************************//

//***************************************************************************//
Console::Console()
{
  //-------------------------------------------------------------------------//
  // Create Console
  AllocConsole();
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Set handles to standard input, output and error
  mhStdError   = GetStdHandle(STD_ERROR_HANDLE);
  mhStdInput   = GetStdHandle(STD_INPUT_HANDLE);
  mhStdOutput  = GetStdHandle(STD_OUTPUT_HANDLE);
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Display message box if we could not get input, output or error handles
  if (mhStdInput  == INVALID_HANDLE_VALUE || 
      mhStdOutput == INVALID_HANDLE_VALUE || 
      mhStdError  == INVALID_HANDLE_VALUE ) 
  {
    MessageBox(NULL, "GetStdHandle", "Console Error", MB_OK);
  }
  //-------------------------------------------------------------------------//
}
//***************************************************************************//

//***************************************************************************//
Console::~Console()
{
  //-------------------------------------------------------------------------//
  // Release Console
  FreeConsole();
  //-------------------------------------------------------------------------//
}
//***************************************************************************//

//***************************************************************************//
void Console::SetTitle(char* cpConsoleTitle)
{
  //-------------------------------------------------------------------------//
  // Set title of console
  SetConsoleTitle(cpConsoleTitle);
  //-------------------------------------------------------------------------//
}
//***************************************************************************//

//***************************************************************************//
long Console::Read(char* cpInputbuffer, const unsigned short cusBufferSize)
{
  //-------------------------------------------------------------------------//
  // This assertaion is thrown if cusBufferSize is larger than MAX_CONSOLE_INPUT_BUFFER_SIZE
  // The buffer size must be smaller then MAX_CONSOLE_INPUT_BUFFER_SIZE or 
  // ReadConsole will fail
  assert(cusBufferSize <= MAX_CONSOLE_INPUT_BUFFER_SIZE);
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Create a pionter and some space so that ReadConsole can set the size of 
  // buffer read
  LPDWORD pNumberOfBytesRead = new DWORD;
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Zero out buffer
  memset(cpInputbuffer, 0, cusBufferSize);
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Read from the console this function will block until the enter key is pressed
  ReadConsole(mhStdInput, (void*)cpInputbuffer, cusBufferSize, pNumberOfBytesRead, NULL);
  //-------------------------------------------------------------------------//
  
  //-------------------------------------------------------------------------//
  // Store the number of bytes read in to a temp variable so that it can be returned
  // ans destroy pNumberOfBytesRead craeted above
  int iReturnValue = (int)*pNumberOfBytesRead;
  delete pNumberOfBytesRead;
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Return the Number of bytes read
  return iReturnValue;
  //-------------------------------------------------------------------------//
}
//***************************************************************************//

//***************************************************************************//
long Console::Write(const void* cpOutputBuffer, unsigned short usBufferSize, const bool bWriteToStdError)
{
  //-------------------------------------------------------------------------//
  // The default of usBufferSize is zero.  If this is value passed assume a NULL
  // terminated string is in cpOutputBuffer and set the buffer lenght to the 
  // length of that string
  if(usBufferSize == 0)
    usBufferSize = (unsigned short)strlen((char*)cpOutputBuffer);
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Used to get the number of bytes written
  DWORD numWritten;
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // If bWriteToStdError is true; write to standard error otherwise write to 
  // standard output
  if(bWriteToStdError)
    WriteConsole(mhStdOutput, cpOutputBuffer, usBufferSize, &numWritten, NULL);
  else
    WriteConsole(mhStdError, cpOutputBuffer, usBufferSize, &numWritten, NULL);
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Return the number of bytes written
  return (int)numWritten;
  //-------------------------------------------------------------------------//
}
//***************************************************************************//
