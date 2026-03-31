package com.example.bookmarkmanager.service;

import com.example.bookmarkmanager.dto.LoginRequestDto;
import com.example.bookmarkmanager.dto.LoginResponseDto;
import com.example.bookmarkmanager.dto.UserRegistrationDto;
import com.example.bookmarkmanager.model.User;
import com.example.bookmarkmanager.repository.UserRepository;
import com.example.bookmarkmanager.security.JwtTokenProvider;
import java.util.Optional;
import org.junit.Test;
import org.junit.jupiter.api.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.mockito.junit.MockitoJUnitRunner;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.security.authentication.AuthenticationManager;
import org.springframework.security.authentication.BadCredentialsException;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContext;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.crypto.password.PasswordEncoder;
import static org.junit.jupiter.api.Assertions.*;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.mockito.Mockito.*;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

@RunWith(MockitoJUnitRunner.class)
@SpringBootTest
public class UserServiceTest {

    @Mock
    private Authentication authentication;

    @Mock
    private AuthenticationManager authenticationManager;

    @Mock
    private JwtTokenProvider jwtTokenProvider;

    @Mock
    private PasswordEncoder passwordEncoder;

    @Mock
    private SecurityContext securityContext;

    @Mock
    private UserRepository userRepository;

    @InjectMocks
    private UserService userService;

    /**
     * Test case for authenticating a user with invalid credentials.
     * This test verifies that the authenticateUser method throws a BadCredentialsException
     * when provided with incorrect username or password.
     */
    @Test
    public void testAuthenticateUser_InvalidCredentials() {
        // Mock dependencies
        AuthenticationManager authenticationManager = mock(AuthenticationManager.class);
        JwtTokenProvider jwtTokenProvider = mock(JwtTokenProvider.class);

        // Create UserService instance
        UserService userService = new UserService(null, null, authenticationManager, jwtTokenProvider);

        // Create login request with invalid credentials
        LoginRequestDto loginRequest = new LoginRequestDto("invalidUser", "wrongPassword");

        // Mock authentication failure
        when(authenticationManager.authenticate(any(UsernamePasswordAuthenticationToken.class)))
                .thenThrow(new BadCredentialsException("Invalid username or password"));

        // Assert that BadCredentialsException is thrown
        assertThrows(BadCredentialsException.class, () -> userService.authenticateUser(loginRequest));
    }

    /**
     * Test case for getCurrentUser method when the user is found in the repository.
     * It verifies that the method returns the correct user when authentication is present
     * and the user exists in the repository.
     */
    @Test
    public void testGetCurrentUserWhenUserFound() {
        // Arrange
        String username = "testuser";
        User expectedUser = new User(username, "test@example.com", "password");

        when(SecurityContextHolder.getContext()).thenReturn(securityContext);
        when(securityContext.getAuthentication()).thenReturn(authentication);
        when(authentication.getName()).thenReturn(username);
        when(userRepository.findByUsername(username)).thenReturn(Optional.of(expectedUser));

        // Act
        User result = userService.getCurrentUser();

        // Assert
        assertNotNull(result);
        assertEquals(expectedUser, result);
        verify(userRepository).findByUsername(username);
    }

    /**
     * Tests that the UserService constructor throws a NullPointerException when
     * any of its required dependencies are null.
     */
    @Test(expected = NullPointerException.class)
    public void testUserServiceConstructorWithNullDependencies() {
        new UserService(null, passwordEncoder, authenticationManager, jwtTokenProvider);
    }

    /**
     * Test case for UserService constructor
     * Verifies that the UserService object is created successfully with all dependencies
     */
    @Test
    public void test_UserService_ConstructorInitialization() {
        UserService userService = new UserService(userRepository, passwordEncoder, authenticationManager, jwtTokenProvider);
        // Assert that userService is not null, indicating successful object creation
        assertNotNull(userService);
    }

    /**
     * Test case for authenticateUser method when valid credentials are provided.
     * It verifies that the method returns a LoginResponseDto with the correct JWT token,
     * username, and expiration time.
     */
    @Test
    public void test_authenticateUser_validCredentials() {
        // Mocking dependencies
        AuthenticationManager authenticationManager = mock(AuthenticationManager.class);
        JwtTokenProvider jwtTokenProvider = mock(JwtTokenProvider.class);
        UserService userService = new UserService(null, null, authenticationManager, jwtTokenProvider);

        // Preparing test data
        String username = "testuser";
        String password = "password123";
        String jwt = "testJwtToken";
        long expirationTime = 3600000L;

        LoginRequestDto loginRequest = new LoginRequestDto(username, password);

        // Mocking authentication process
        Authentication authentication = mock(Authentication.class);
        UserDetails userDetails = mock(UserDetails.class);
        when(authenticationManager.authenticate(new UsernamePasswordAuthenticationToken(username, password)))
                .thenReturn(authentication);
        when(authentication.getPrincipal()).thenReturn(userDetails);
        when(userDetails.getUsername()).thenReturn(username);
        when(jwtTokenProvider.generateToken(userDetails)).thenReturn(jwt);
        when(jwtTokenProvider.getExpirationTime()).thenReturn(expirationTime);

        // Calling the method under test
        LoginResponseDto response = userService.authenticateUser(loginRequest);

        // Assertions
        assertEquals(jwt, response.getToken());
        assertEquals(username, response.getUsername());
        assertEquals(expirationTime, response.getExpirationTime());
    }

    /**
     * Tests the getCurrentUser method when the user is not found in the repository.
     * This test verifies that a RuntimeException is thrown when the user associated
     * with the authentication is not present in the database.
     */
    @Test
    public void test_getCurrentUser_userNotFound() {
        // Mock dependencies
        UserRepository userRepository = mock(UserRepository.class);
        Authentication authentication = mock(Authentication.class);
        SecurityContext securityContext = mock(SecurityContext.class);

        // Set up SecurityContextHolder
        SecurityContextHolder.setContext(securityContext);
        when(securityContext.getAuthentication()).thenReturn(authentication);

        // Set up authentication
        when(authentication.getName()).thenReturn("nonexistentUser");

        // Set up userRepository to return empty Optional
        when(userRepository.findByUsername("nonexistentUser")).thenReturn(Optional.empty());

        // Create UserService instance
        UserService userService = new UserService(userRepository, null, null, null);

        // Assert that RuntimeException is thrown
        assertThrows(RuntimeException.class, () -> userService.getCurrentUser());
    }

    /**
     * Test case for registering a user with a username that already exists.
     * This test verifies that the registerUser method throws a RuntimeException
     * when attempting to register a user with a username that is already taken.
     */
    @Test
    public void test_registerUser_usernameAlreadyTaken() {
        // Arrange
        UserRegistrationDto registrationDto = new UserRegistrationDto();
        registrationDto.setUsername("existingUser");
        registrationDto.setPassword("password123");

        when(userRepository.existsByUsername("existingUser")).thenReturn(true);

        // Act & Assert
        assertThrows(RuntimeException.class, () -> userService.registerUser(registrationDto));
    }

    /**
     * Test case for registerUser method when the username already exists.
     * This test verifies that a RuntimeException is thrown when attempting to register a user with an existing username.
     */
    @Test
    public void test_registerUser_whenUsernameAlreadyExists() {
        MockitoAnnotations.openMocks(this);

        UserRegistrationDto registrationDto = new UserRegistrationDto();
        registrationDto.setUsername("existingUser");
        registrationDto.setPassword("password123");

        when(userRepository.existsByUsername("existingUser")).thenReturn(true);

        assertThrows(RuntimeException.class, () -> {
            userService.registerUser(registrationDto);
        });

        verify(userRepository).existsByUsername("existingUser");
        verifyNoMoreInteractions(userRepository);
        verifyNoInteractions(passwordEncoder);
    }

    /**
     * Test case for registering a new user when the username is not already taken.
     * This test verifies that the user is successfully registered when the username is available.
     */
    @Test
    public void test_registerUser_whenUsernameIsNotTaken() {
        // Arrange
        UserRegistrationDto registrationDto = new UserRegistrationDto("newuser", "password123");
        when(userRepository.existsByUsername("newuser")).thenReturn(false);
        when(passwordEncoder.encode("password123")).thenReturn("encodedPassword");

        // Act
        userService.registerUser(registrationDto);

        // Assert
        verify(userRepository).existsByUsername("newuser");
        verify(passwordEncoder).encode("password123");
        verify(userRepository).save(any(User.class));
    }

}
