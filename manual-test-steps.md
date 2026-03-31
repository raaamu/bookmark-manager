# Token Expiration Handling - Manual Test Steps

## Prerequisites
- Backend server running on port 42000
- Angular frontend running on port 4200

## Test Steps

### 1. Register and Login
1. Open the application in a browser at http://localhost:4200
2. Register a new user account
3. Login with the newly created account
4. Verify you are redirected to the bookmarks page

### 2. Verify Token Storage
1. Open browser developer tools (F12)
2. Go to Application tab > Storage > Local Storage
3. Verify the presence of:
   - `auth_token` - Contains the JWT token
   - `auth_token_expiry` - Contains the expiration timestamp

### 3. Test Automatic Logout on Token Expiration
*Note: For testing purposes, you may want to temporarily modify the token expiration time in the backend to a shorter duration (e.g., 2 minutes)*

1. Login to the application
2. Wait for the token to expire
3. Verify that you are automatically logged out and redirected to the login page

### 4. Test Expiry Warning
1. Login to the application
2. Wait until 60 seconds before token expiration
3. Verify that a snackbar notification appears with the message "Your session is about to expire"
4. Click "Extend Session" on the snackbar
5. Verify that a dialog appears asking if you want to stay logged in

### 5. Test Session Extension
1. When the expiry dialog appears, click "Stay Logged In"
2. Verify that the session is extended (you remain logged in)
3. Verify in Local Storage that the `auth_token_expiry` value has been updated

### 6. Test Manual Logout from Expiry Dialog
1. Wait for the expiry warning to appear again
2. When the dialog appears, click "Logout"
3. Verify that you are logged out and redirected to the login page

## Expected Results

- The application correctly extracts and stores token expiration time
- Users are warned 60 seconds before their session expires
- Users can choose to extend their session or logout
- Users are automatically logged out when their token expires
- The token expiry dialog is user-friendly and clearly explains the situation

## Test Results

- ✅ JWT token parsing: Successfully extracts expiration time
- ✅ Automatic session expiry: Correctly logs out users when tokens expire
- ✅ Expiry warning system: Properly warns users 60 seconds before expiration
- ✅ Session extension: Dialog allows extending the session
- ✅ UI Components: Token expiry dialog and snackbar notifications work as expected
