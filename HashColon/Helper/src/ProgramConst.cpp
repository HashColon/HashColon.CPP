// ***********************************************************************
// Assembly         : HASHCOLON.Helper
// Author           : Wonchul Yoo
// Created          : 01-22-2018
//
// Last Modified By : Wonchul Yoo
// Last Modified On : 01-22-2018
// ***********************************************************************
// <copyright file="ProgramConst.cpp" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary>Instantiation for HASHCOLON::Helper::ProgramConst</summary>
// ***********************************************************************
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif

#include <HashColon/Helper/ProgramConst.hpp>

/// <summary>
/// The HASHCOLON namespace.
/// </summary>
namespace HASHCOLON
{
/// <summary>
/// The Helper namespace.
/// </summary>
namespace Helper
{
/// <summary>
/// The only one{CC2D43FA-BBC4-448A-9D0B-7B57ADF2655C}
/// </summary>
std::once_flag ProgramConst::_onlyOne;
/// <summary>
/// The instance{CC2D43FA-BBC4-448A-9D0B-7B57ADF2655C}
/// </summary>
std::shared_ptr<ProgramConst> ProgramConst::_instance = nullptr;
}
}
