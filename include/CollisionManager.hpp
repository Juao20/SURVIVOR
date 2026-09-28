#pragma once

#include <raylib.h>
#include <vector>
#include <memory>
#include <cmath>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"

class CollisionManager
{
public:
    CollisionManager() = default;
    ~CollisionManager() = default;

    static bool checkPlayerWallCollision(const Rectangle& playerBox, const Rectangle& wall);
    static bool checkProjectileWallCollision(const Vector2& projectilePos, float radius, const Rectangle& wall);
    static bool checkProjectileEnemyCollision(const Vector2& projectilePos, float radius, const Rectangle& enemyBox);
    static bool checkPlayerPickupCollision(const Rectangle& playerBox, const Vector2& pickupPos, float pickupRadius = 15.0f);
    static bool checkPlayerEnemyCollision(const Rectangle& playerBox, const Rectangle& enemyBox);
    static bool checkHitscanEnemyCollision(const Vector2& rayStart, const Vector2& rayEnd,
                                           const Rectangle& enemyBox, Vector2& hitPoint);
    static bool checkCircleRectCollision(const Vector2& circlePos, float radius, const Rectangle& rect);
    static bool checkLineRectCollision(const Vector2& lineStart, const Vector2& lineEnd, const Rectangle& rect);
};

bool CollisionManager::checkPlayerWallCollision(const Rectangle& playerBox, const Rectangle& wall)
{
    return CheckCollisionRecs(playerBox, wall);
}

bool CollisionManager::checkProjectileWallCollision(const Vector2& projectilePos, float radius, const Rectangle& wall)
{
    return checkCircleRectCollision(projectilePos, radius, wall);
}

bool CollisionManager::checkProjectileEnemyCollision(const Vector2& projectilePos, float radius, const Rectangle& enemyBox)
{
    return checkCircleRectCollision(projectilePos, radius, enemyBox);
}

bool CollisionManager::checkPlayerPickupCollision(const Rectangle& playerBox, const Vector2& pickupPos, float pickupRadius)
{
    Vector2 playerCenter = { playerBox.x + playerBox.width / 2, playerBox.y + playerBox.height / 2 };
    float distance = sqrtf(
        (playerCenter.x - pickupPos.x) * (playerCenter.x - pickupPos.x) +
        (playerCenter.y - pickupPos.y) * (playerCenter.y - pickupPos.y));
    return distance < (pickupRadius + playerBox.width / 2);
}

bool CollisionManager::checkPlayerEnemyCollision(const Rectangle& playerBox, const Rectangle& enemyBox)
{
    return CheckCollisionRecs(playerBox, enemyBox);
}

bool CollisionManager::checkHitscanEnemyCollision(const Vector2& rayStart, const Vector2& rayEnd,
                                                   const Rectangle& enemyBox, Vector2& hitPoint)
{
    if (checkLineRectCollision(rayStart, rayEnd, enemyBox)) {
        hitPoint.x = enemyBox.x + enemyBox.width  / 2;
        hitPoint.y = enemyBox.y + enemyBox.height / 2;
        return true;
    }
    return false;
}

bool CollisionManager::checkCircleRectCollision(const Vector2& circlePos, float radius, const Rectangle& rect)
{
    float closestX = (circlePos.x < rect.x) ? rect.x :
                     (circlePos.x > rect.x + rect.width) ? rect.x + rect.width : circlePos.x;
    float closestY = (circlePos.y < rect.y) ? rect.y :
                     (circlePos.y > rect.y + rect.height) ? rect.y + rect.height : circlePos.y;
    float dx = circlePos.x - closestX;
    float dy = circlePos.y - closestY;
    return (dx * dx + dy * dy) < (radius * radius);
}

bool CollisionManager::checkLineRectCollision(const Vector2& lineStart, const Vector2& lineEnd, const Rectangle& rect)
{
    Vector2 hitPoint;
    if (CheckCollisionLines(lineStart, lineEnd, {rect.x, rect.y}, {rect.x, rect.y + rect.height}, &hitPoint)) return true;
    if (CheckCollisionLines(lineStart, lineEnd, {rect.x + rect.width, rect.y}, {rect.x + rect.width, rect.y + rect.height}, &hitPoint)) return true;
    if (CheckCollisionLines(lineStart, lineEnd, {rect.x, rect.y}, {rect.x + rect.width, rect.y}, &hitPoint)) return true;
    if (CheckCollisionLines(lineStart, lineEnd, {rect.x, rect.y + rect.height}, {rect.x + rect.width, rect.y + rect.height}, &hitPoint)) return true;
    return false;
}
