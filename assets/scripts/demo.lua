local t = 0.0

function update(dt)
    t = t + dt

    local x = 160.0 + math.cos(t * 2.0) * 56.0
    local y = 106.0 + math.sin(t * 3.0) * 28.0

    return x, y, string.format("LuaJIT update: t=%.2f", t)
end
