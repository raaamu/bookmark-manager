FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base
WORKDIR /app
EXPOSE 42000

FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
WORKDIR /src
COPY ["BookmarkManager.csproj", "./"]
RUN dotnet restore "BookmarkManager.csproj"
COPY . .
RUN dotnet build "BookmarkManager.csproj" -c Release -o /app/build

FROM build AS publish
RUN dotnet publish "BookmarkManager.csproj" -c Release -o /app/publish

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "BookmarkManager.dll"]
