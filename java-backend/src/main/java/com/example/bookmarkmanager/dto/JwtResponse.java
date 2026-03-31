package com.example.bookmarkmanager.dto;

/**
 * Response object containing JWT token
 */
public class JwtResponse {
    
    private String token;
    private String username;
    private long expiresIn;
    
    public JwtResponse() {
    }
    
    public JwtResponse(String token, String username, long expiresIn) {
        this.token = token;
        this.username = username;
        this.expiresIn = expiresIn;
    }
    
    public String getToken() {
        return token;
    }
    
    public void setToken(String token) {
        this.token = token;
    }
    
    public String getUsername() {
        return username;
    }
    
    public void setUsername(String username) {
        this.username = username;
    }
    
    public long getExpiresIn() {
        return expiresIn;
    }
    
    public void setExpiresIn(long expiresIn) {
        this.expiresIn = expiresIn;
    }
}
