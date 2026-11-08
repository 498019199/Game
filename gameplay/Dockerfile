FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
WORKDIR /src/gameplay
COPY gameplay/ ./
COPY zengine/assets/config/ /src/config/
RUN dotnet publish server/Game.Server.csproj -c Release -o /app --nologo \
    && dotnet /app/Game.Server.dll --bake /src/config /app/game.catalog.pb

FROM mcr.microsoft.com/dotnet/runtime:10.0 AS runtime
WORKDIR /app
COPY --from=build /app/ ./
USER $APP_UID
EXPOSE 7777
ENTRYPOINT ["dotnet", "Game.Server.dll"]
CMD ["--listen", "0.0.0.0", "7777"]
