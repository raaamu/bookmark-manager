package com.example.bookmarkmanager.controller;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.HashMap;
import java.util.Map;

/**
 * Controller for health check endpoints
 */
@RestController
public class HealthController {

    /**
     * Simple health check endpoint
     * @return Health status
     */
    @GetMapping("/health")
    public ResponseEntity<Map<String, String>> healthCheck() {
        Map<String, String> response = new HashMap<>();
        response.put("status", "UP");
        response.put("service", "Bookmark Manager Java Backend");
        return ResponseEntity.ok(response);
    }
}
