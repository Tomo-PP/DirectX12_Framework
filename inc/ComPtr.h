#pragma once
/*****************************************************************
 * Include
 *****************************************************************/
#include <wrl/client.h>


/*****************************************************************
 * using ComPtr
 ****************************************************************/
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;