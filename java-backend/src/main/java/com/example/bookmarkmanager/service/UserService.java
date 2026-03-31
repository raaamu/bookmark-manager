package com.example.bookmarkmanager.service;

import com.example.bookmarkmanager.dto.LoginRequestDto;
import com.example.bookmarkmanager.dto.LoginResponseDto;
import com.example.bookmarkmanager.dto.UserRegistrationDto;
import com.example.bookmarkmanager.model.User;
import com.example.bookmarkmanager.repository.UserRepository;
import com.example.bookmarkmanager.security.JwtTokenProvider;
import org.springframework.security.authentication.AuthenticationManager;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

@Service
public class UserService {

    private final UserRepository userRepository;
    private final PasswordEncoder passwordEncoder;
    private final AuthenticationManager authenticationManager;
    private final JwtTokenProvider jwtTokenProvider;

    public UserService(UserRepository userRepository, PasswordEncoder passwordEncoder,
                       AuthenticationManager authenticationManager, JwtTokenProvider jwtTokenProvider) {
        this.userRepository = userRepository;
        this.passwordEncoder = passwordEncoder;
        this.authenticationManager = authenticationManager;
        this.jwtTokenProvider = jwtTokenProvider;
    }

    public void registerUser(UserRegistrationDto registrationDto) {
        if (userRepository.existsByUsername(registrationDto.getUsername())) {
            throw new RuntimeException("Username is already taken");
        }

        User user = new User(
                registrationDto.getUsername(),
                "default@example.com", // Adding default email since it's required
                passwordEncoder.encode(registrationDto.getPassword())
        );

        userRepository.save(user);
    }

    public LoginResponseDto authenticateUser(LoginRequestDto loginRequest) {
        Authentication authentication = authenticationManager.authenticate(
                new UsernamePasswordAuthenticationToken(loginRequest.getUsername(), loginRequest.getPassword())
        );

        SecurityContextHolder.getContext().setAuthentication(authentication);
        UserDetails userDetails = (UserDetails) authentication.getPrincipal();
        String jwt = jwtTokenProvider.generateToken(userDetails);
        
        return new LoginResponseDto(jwt, userDetails.getUsername(), jwtTokenProvider.getExpirationTime());
    }

    public User getCurrentUser() {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();
        return userRepository.findByUsername(username)
                .orElseThrow(() -> new RuntimeException("User not found"));
    }
}
