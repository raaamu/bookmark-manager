use axum::{routing::{get, post, delete}, Router};
use clap::Parser;
use sqlx::sqlite::{SqlitePool, SqlitePoolOptions};
use std::{net::SocketAddr, sync::Arc};
use tokio::signal;
use tracing::info;
use tracing_subscriber::{layer::SubscriberExt, util::SubscriberInitExt, EnvFilter};

mod config;
mod db;
mod errors;
mod handlers;
mod models;
mod auth; // Make sure this line is present

use config::Config;

// Shared application state
#[derive(Clone)]
pub struct AppState {
    db_pool: SqlitePool,
    config: Arc<Config>,
}

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Port number to listen on
    #[arg(short, long, default_value_t = 8080)]
    port: u16,
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    dotenvy::dotenv().ok();
    // Initialize logging
    tracing_subscriber::registry()
        .with(EnvFilter::try_from_default_env().unwrap_or_else(|_| "info".into()))
        .with(tracing_subscriber::fmt::layer())
        .init();

    let args = Args::parse();

    // Load configuration (including JWT secret generation if needed)
    let config = Arc::new(Config::from_env().await?);

    // Initialize database connection pool
    let db_pool = SqlitePoolOptions::new()
        .max_connections(5) // Configure as needed
        .connect(&config.database_url)
        .await?;
    info!("Database connection pool established.");

    // Run database migrations (creates tables if they don't exist)
    sqlx::migrate!("./migrations") // Assumes migrations are in a 'migrations' folder
        .run(&db_pool)
        .await?;
    info!("Database migrations applied.");


    // Create shared application state
    let app_state = AppState {
        db_pool,
        config,
    };

    // Define application routes
    // TODO: Add authentication middleware to protected routes
    let app = Router::new()
        .route("/register", post(handlers::register_user))
        .route("/login", post(handlers::login_user))
        .route("/bookmarks", get(handlers::get_bookmarks).post(handlers::add_bookmark))
        .route("/bookmarks/search", get(handlers::search_bookmarks))
        .route("/bookmarks/:id", delete(handlers::delete_bookmark))
        .with_state(app_state);

    // Start the server
    let addr = SocketAddr::from(([0, 0, 0, 0], args.port));
    info!("Bookmark Manager API server running on {}", addr);

    let listener = tokio::net::TcpListener::bind(addr).await.unwrap();
    axum::serve(listener, app).await.unwrap();

    info!("Server shut down gracefully.");
    Ok(())
}

// Graceful shutdown handler
async fn shutdown_signal() {
    let ctrl_c = async {
        signal::ctrl_c()
            .await
            .expect("Failed to install Ctrl+C handler");
    };

    #[cfg(unix)]
    let terminate = async {
        signal::unix::signal(signal::unix::SignalKind::terminate())
            .expect("Failed to install signal handler")
            .recv()
            .await;
    };

    #[cfg(not(unix))]
    let terminate = std::future::pending::<()>();

    tokio::select! {
        _ = ctrl_c => {},
        _ = terminate => {},
    }

    info!("Signal received, starting graceful shutdown...");
}
