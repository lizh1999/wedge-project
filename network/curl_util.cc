#include "curl_util.h"

namespace wedge {

expected<CurlGlobal, CurlError> CurlGlobal::init() {
  CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (res != CURLE_OK) {
    return unexpected(CurlError(res));
  }
  return expected<CurlGlobal, CurlError>(CurlGlobal());
}

CurlGlobal::~CurlGlobal() { curl_global_cleanup(); }

}