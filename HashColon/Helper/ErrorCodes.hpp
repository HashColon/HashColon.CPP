// ***********************************************************************
// Assembly         : HASHCOLON.Helper
// Author           : Wonchul Yoo
// Created          : 03-02-2016
//
// Last Modified By : Wonchul Yoo
// Last Modified On : 01-21-2016
// ***********************************************************************
// <copyright file="ErrorCodes.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary>Error code definitions for HASHCOLONLib</summary>
// ***********************************************************************
#ifndef HASHCOLON_HELPER_ERRORCODES_HPP
#define HASHCOLON_HELPER_ERRORCODES_HPP

#define _HASHCOLONErrCode_NULL 0
#define _HASHCOLONErrCode_OK 1
#define _HASHCOLONErrCode_ERR -1
#define _HASHCOLONErrCode_ERR_WRONGOPTION -2

#define _HASHCOLONErrCheck(func)      \
  {                               \
    int _HASHCOLONErrCode_TMP = func; \
    if (_HASHCOLONErrCode_TMP < 0) {  \
      return _HASHCOLONErrCode_ERR;   \
    }                             \
  }

#define _HASHCOLONIfErr(func) if (func < 0)

#endif  // HASHCOLON_HELPER_ERRORCODES_H
