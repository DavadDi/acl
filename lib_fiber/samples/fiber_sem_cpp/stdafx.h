// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once


//#include <iostream>
//#include <tchar.h>

// TODO: �ڴ˴����ó���Ҫ��ĸ���ͷ�ļ�

#include "lib_acl.h"
#include "acl_cpp/lib_acl.hpp"
#include "fiber/libfiber.hpp"

#ifdef	ACL_USE_CPP11
#include "fiber/go_fiber.hpp"
#endif

#ifdef	WIN32
#define	snprintf _snprintf
#endif
