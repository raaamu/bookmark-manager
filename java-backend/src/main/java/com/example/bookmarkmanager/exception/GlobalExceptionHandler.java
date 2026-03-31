package com.example.bookmarkmanager.exception;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.BadCredentialsException;
import org.springframework.validation.FieldError;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import java.util.HashMap;
import java.util.Map;

/**
 * Global exception handler for the application
 */
@RestControllerAdvice
public class GlobalExceptionHandler {

    /**
     * Handle validation exceptions
     * 
     * @param ex the validation exception
     * @return ResponseEntity with validation errors
     */
    @ExceptionHandler(MethodArgumentNotValidException.class)
    public ResponseEntity<Map<String, String>> handleValidationExceptions(MethodArgumentNotValidException ex) {
        Map<String, String> errors = new HashMap<>();
        ex.getBindingResult().getAllErrors().forEach(error -> {
            String fieldName = ((FieldError) error).getField();
            String errorMessage = error.getDefaultMessage();
            errors.put(fieldName, errorMessage);
        });
        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(errors);
    }

    /**
     * Handle authentication exceptions
     * 
     * @param ex the authentication exception
     * @return ResponseEntity with error message
     */
    @ExceptionHandler(BadCredentialsException.class)
    public ResponseEntity<Map<String, String>> handleAuthenticationException(BadCredentialsException ex) {
        Map<String, String> error = new HashMap<>();
        error.put("message", "Invalid username or password");
        return ResponseEntity.status(HttpStatus.UNAUTHORIZED).body(error);
    }

    /**
     * Handle general exceptions
     * 
     * @param ex the exception
     * @return ResponseEntity with error message
     */
    @ExceptionHandler(Exception.class)
    public ResponseEntity<Map<String, String>> handleGeneralExceptions(Exception ex) {
        Map<String, String> error = new HashMap<>();
        error.put("message", ex.getMessage());
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(error);
    }
}
