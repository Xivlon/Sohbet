# CORS & Routing Fix for Academic Social Server

## Problems Identified

### 1. **OPTIONS Requests Not Handled Before Body Logging**
**Symptom:** Browser sends OPTIONS preflight, server logs show "body length: 0" for OPTIONS requests calling `handleCreatePost()`

**Root Cause:** The debug logging code was trying to access `request.body.substr(0, 200)` before checking if the request method was OPTIONS. OPTIONS requests have no body, but the code wasn't handling this.

**Impact:** 
- OPTIONS preflight responses weren't being generated properly
- Browser receives incomplete/incorrect CORS headers
- Browser never sends the actual POST request

### 2. **Incorrect Debug Message**
The debug log said "DEBUG: handleCreatePost called" for ALL requests, not just POST requests.

---

## The Fix

### Fixed `handleRequest()` function:

```cpp
HttpResponse AcademicSocialServer::handleRequest(const HttpRequest& request) {
    // Extract base path (without query string)
    std::string base_path = request.path;
    size_t query_pos = base_path.find('?');
    if (query_pos != std::string::npos) {
        base_path = base_path.substr(0, query_pos);
    }
    
    // ✅ Handle CORS preflight requests FIRST (before logging/processing body)
    if (request.method == "OPTIONS") {
        std::cerr << "DEBUG: Handling OPTIONS preflight for " << base_path << std::endl;
        return HttpResponse(200, "text/plain", "");
    }
    
    // ✅ Now safe to log debug info for actual requests
    std::cerr << "DEBUG: handleRequest called" << std::endl;
    std::cerr << "  Method: " << request.method << std::endl;
    std::cerr << "  Path: " << request.path << std::endl;
    std::cerr << "  Base Path: " << base_path << std::endl;
    std::cerr << "  Headers count: " << request.headers.size() << std::endl;
    std::cerr << "  Body length: " << request.body.length() << std::endl;
    // ✅ Only try to access body if it exists
    if (request.body.length() > 0) {
        std::cerr << "  Body content (first 200 chars): " << request.body.substr(0, 200) << std::endl;
    }
    
    int author_id = getUserIdFromAuth(request);
    
    // ... rest of routing logic unchanged
}
```

### Key Changes:

1. **Move OPTIONS check to the very first thing** - before any logging
2. **Add guard before accessing body** - only call `substr()` if body exists
3. **Fix debug message** - say "handleRequest" not "handleCreatePost"
4. **Maintain CORS headers** - the `formatHttpResponse()` already includes them

---

## How CORS Flow Works Now

### Browser makes POST request:

```
1. Browser sends OPTIONS preflight:
   OPTIONS /api/posts HTTP/1.1
   Origin: https://yourapp.com
   Access-Control-Request-Method: POST
   
2. Server detects OPTIONS method
   → Returns immediately with 200 OK + CORS headers
   → Log: "DEBUG: Handling OPTIONS preflight for /api/posts"
   
3. Browser receives CORS headers ✓
   → Proceeds to send actual POST
   
4. Server receives POST:
   POST /api/posts HTTP/1.1
   Content-Type: application/json
   Authorization: Bearer [token]
   
   {"content": "..."}
   
5. Server routes to handleCreatePost()
   → Creates post ✓
```

---

## CORS Headers Being Sent

The `formatHttpResponse()` function adds these headers to **all** responses:

```
Access-Control-Allow-Origin: [echo origin or *]
Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS
Access-Control-Allow-Headers: Content-Type, Authorization
Access-Control-Allow-Credentials: true (if origin != *)
```

These are sent with:
- OPTIONS responses (preflight)
- All POST/PUT/DELETE responses
- All GET responses

---

## Expected Logs After Fix

### Browser POSTs to `/api/posts`:

```
DEBUG: handleClient started
DEBUG: parseHttpRequest called with 549 bytes
Headers end at position: 545
Body starts at: 549
Total body bytes: 4         ← Changed! Now shows actual body size
DEBUG: Handling OPTIONS preflight for /api/posts
[Browser receives 200 OK + CORS headers]

DEBUG: handleClient started
DEBUG: parseHttpRequest called with [body_size] bytes
DEBUG: handleRequest called
  Method: POST
  Path: /api/posts
  Base Path: /api/posts
  Headers count: 16
  Body length: [actual_body_size]
  Body content (first 200 chars): {"content": "..."}
```

---

## Deployment Steps

1. Replace the old `server.cpp` with the fixed version
2. Recompile: `g++ ... server.cpp ...`
3. Redeploy to Fly.io
4. Try making a POST request from your frontend
5. Check logs - you should now see:
   - OPTIONS preflight being handled
   - Actual POST request coming through with body
   - No more CORS errors in browser

---

## Testing

### Test with curl:

```bash
# Test OPTIONS (preflight)
curl -X OPTIONS http://localhost:8080/api/posts \
  -H "Origin: http://localhost:3000" \
  -H "Access-Control-Request-Method: POST" \
  -H "Access-Control-Request-Headers: Content-Type" \
  -v

# Should see 200 OK with CORS headers

# Test actual POST
curl -X POST http://localhost:8080/api/posts \
  -H "Origin: http://localhost:3000" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer [token]" \
  -d '{"content": "test"}' \
  -v

# Should create post successfully
```

### Test with browser DevTools:

1. Open DevTools → Network tab
2. Look for preflight requests (OPTIONS)
3. Check Response headers for `Access-Control-Allow-*`
4. Check actual POST request comes after OPTIONS
5. Verify no "CORS blocked" errors
