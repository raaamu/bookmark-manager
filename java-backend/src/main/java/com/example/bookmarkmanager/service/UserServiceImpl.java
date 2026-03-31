package com.example.bookmarkmanager.service;

import com.example.bookmarkmanager.model.User;
import com.example.bookmarkmanager.repository.UserRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.transaction.annotation.Transactional;

import java.util.ArrayList;

// Moved from UserDetailsServiceImpl to avoid conflict with security/UserDetailsServiceImpl
public class UserServiceImpl {
    @Autowired
    UserRepository userRepository;

    @Transactional
    public UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {
        User user = userRepository.findByUsername(username)
                .orElseThrow(() -> new UsernameNotFoundException("User Not Found with username: " + username));

        return new org.springframework.security.core.userdetails.User(
                user.getUsername(),
                user.getPassword(),
                new ArrayList<>());
    }
}
