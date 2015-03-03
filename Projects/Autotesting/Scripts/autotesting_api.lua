TIMEOUT = 30.0 -- Big time out for waiting
TIMECLICK = 0.5 -- time for simple action
DELAY = 0.5 -- time for simulation of human reaction

MULTIPLAYER_TIMEOUT_COUNT = 300 -- Multiplayer timeout

EPSILON = 1

MAX_LIST_COUNT = 100 -- Max item of list

-- API setup
function SetPackagePath(path)
    package.path = package.path .. ";" .. path .. "Actions/?.lua;" .. path .. "Scripts/?.lua;"
    require "coxpcall"
end

function assert(isTrue, errorMsg)
    if not isTrue then OnError(tostring(errorMsg)) end
end

local function toboolean(condition)
    return not not condition
end

local function __GetNewPosition(list, vertical, invert, notInCenter)
    local position, newPosition = Vector.Vector2(), Vector.Vector2()
    local rect = GetElementRect(list)
    position.x, position.y = (rect.x + rect.dx / 2), (rect.y + rect.dy / 2)
    if invert then
        newPosition.y = position.y + rect.dy / 2
        newPosition.x = position.x + rect.dx / 2
    else
        newPosition.y = position.y - rect.dy / 2
        newPosition.x = position.x - rect.dx / 2
    end
    if vertical == true then
        newPosition.x = position.x
    else
        newPosition.y = position.y
    end
    if notInCenter then
        newPosition.x = newPosition.x - rect.dx / 8
        position.x = position.x - rect.dx / 8
    end
    return position, newPosition
end

----------------------------------------------------------------------------------------------------
-- High-level test function
----------------------------------------------------------------------------------------------------
-- This function for simple test step without any assertion. Fail while error throwing
-- Parameters:
-- description - step description
-- func - link to step function
-- ... - input parameters for step function
function Step(description, func, ...)
    autotestingSystem:OnStepStart(description)
    Yield()
    local status, err = copcall(func, ...)
    Yield()
    if not status then OnError(err) end
end

-- This function for test step with assertion. Fail when step is returned NIL or FALSE
-- Parameters:
--        description - step description
--        func - link to step function
--        ... - input parameters for step function
function Assert(description, func, ...)
    autotestingSystem:OnStepStart(description)
    Yield()
    local status, err = copcall(func, ...)
    Yield()
    if not status then
        OnError(err)
    elseif not err then
        OnError("Assertion failed, expect true, but function return " .. tostring(err))
    end
end

function AssertNot(description, func, ...)
    autotestingSystem:OnStepStart(description)
    Yield()
    local status, err = copcall(func, ...)
    Yield()
    if not status then
        -- Some error during test step
        OnError(err)
    elseif err then
        OnError("Assertion failed, expect false, but function return " .. tostring(err))
    end
end

function CheckEquals(arg1, arg2)
    if arg1 == arg2 then
        return true
    end
    Log(string.format("'%s' not equal to '%s'", tostring(arg1), tostring(arg2)), "DEBUG")
    return false
end

function Log(message, level)
    level = level or "DEBUG"
    autotestingSystem:Log(level, tostring(message))
    coroutine.yield()
end

function Yield()
    for i = 0, 3 do
        coroutine.yield()
    end
end

function ResumeTest()
    if coroutine.status(co) == "suspended" then
        coroutine.resume(co)
    else
        StopTest()
    end
end

function CreateTest()
    co = coroutine.create(function(func)
        local status, err = copcall(func)
        if not status then
            OnError(err)
        end
    end) -- create a coroutine with foo as the entry
    autotestingSystem = AutotestingSystem.Singleton_Autotesting_Instance()
end

function StartTest(name, test)
    CreateTest()
    autotestingSystem = AutotestingSystem.Singleton_Autotesting_Instance()
    DEVICE_NAME = autotestingSystem:GetDeviceName()
    PLATFORM = autotestingSystem:GetPlatform()
    IS_PHONE = autotestingSystem:IsPhoneScreen()
    autotestingSystem:OnTestStart(name)
    coroutine.resume(co, test)
end

function OnError(text)
    autotestingSystem:OnError(text)
    Yield()
end

function StopTest()
    autotestingSystem:OnTestFinished()
end

-- DB communication
function SaveArchiveToDB(name, archive, document)
    Log(string.format("Save '%s' archive to '%s' document", name, tostring(document)), "Debug")
    autotestingSystem:SaveKeyedArchiveToDB(name, archive, document)
end

function GetParameter(name, default)
    local var = autotestingSystem:GetTestParameter(name)
    if var ~= "not_found" then
        return var
    end
    if testData and testData[name] then
        return testData[name]
    elseif default ~= nil then
        return default
    end
    OnError("Couldn't find value for variable " .. name)
end

function ReadString(name)
    return autotestingSystem:ReadString(name)
end

function WriteString(name, text)
    autotestingSystem:WriteString(name, text)
    coroutine.yield()
end

function MakeScreenshot(skip)
    local skip = skip or false
    print("MakeScreenshot" .. tostring(skip))
    local name = autotestingSystem:MakeScreenshot(skip)
    coroutine.yield()
    return name
end

function GetTimeElapsed()
    return autotestingSystem:GetTimeElapsed()
end

----------------------------------------------------------------------------------------------------
-- Multiplayer API
----------------------------------------------------------------------------------------------------
-- mark current device as ready to work in DB
function ReadState(name)
    return autotestingSystem:ReadState(name)
end

function ReadCommand(name)
    return autotestingSystem:ReadCommand(name)
end

function WriteState(name, state)
    autotestingSystem:WriteState(name, state)
    coroutine.yield()
end

function WriteCommand(name, command)
    autotestingSystem:WriteCommand(name, command)
    coroutine.yield()
end

function InitializeDevice(name)
    DEVICE = name
    Log("Mark " .. name .. " device as Ready")
    Yield()
    autotestingSystem:InitializeDevice(DEVICE)
    Yield()
    WriteState(DEVICE, "ready")
end

function WaitForDevice(name)
    Log("Wait while " .. name .. " device become Ready")
    Yield()
    for i = 1, MULTIPLAYER_TIMEOUT_COUNT do
        if ReadState(name) == "ready" then
            return
        end
        Wait(1)
    end
    OnError("Device " .. name .. " is not ready during timeout")
end

function SendJob(name, command)
    Log("Send to slave " .. name .. " command: " .. command)
    for i = 1, MULTIPLAYER_TIMEOUT_COUNT do
        local state = ReadState(name)
        if state == "ready" then
            WriteCommand(name, command)
            WriteState(name, "wait_execution")
            Log("Device " .. name .. " ready, command was sent")
            return
        elseif state == "error" then
            OnError("Failed to send job to " .. name .. " cause error on device: " .. command)
        end
        Wait(1)
    end
    OnError("Failed to send job to " .. name .. " cause timeout: " .. command)
end

function WaitJob(name)
    Log("Wait for job on slave " .. name)
    for i = 1, MULTIPLAYER_TIMEOUT_COUNT do
        local state = ReadState(name)
        if state == "execution_completed" then
            WriteState(name, "ready")
            Log("Device " .. name .. " finish his job")
            return
        elseif state == "error" then
            OnError("Error on " .. name .. " device")
        else
            Wait(1)
        end
    end
    OnError("Wait for job on " .. name .. " device failed by timeout. Last state " .. state)
end

function SendJobAndWait(name, command)
    SendJob(name, command)
    WaitJob(name)
end

function noneStep()
    Yield()
    return true
end

function GetDeviceName()
    return autotestingSystem:GetDeviceName()
end

function GetPlatform()
    return autotestingSystem:GetPlatform()
end

function IsPhone()
    return autotestingSystem:IsPhoneScreen()
end

------------------------------------------------------------------------------------------------------------------------
-- Work with UI controls
------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------
-- Getters
------------------------------------------------------------------------------------------------------------------------
function GetControl(name)
    local control
    if type(name) == "string" then
        control = autotestingSystem:FindControl(name) or autotestingSystem:FindControlOnPopUp(name)
    else
        control = name
    end
    Yield()
    if control then
        return control
    end
    OnError("Couldn't find control " .. tostring(name))
end

function GetCenter(element)
    local control = GetControl(element)
    if not control then
        OnError("Couldn't find element: " .. element)
    end
    local position = Vector.Vector2()
    local geomData = control:GetGeometricData()
    local rect = geomData:GetUnrotatedRect()
    position.x = rect.x + rect.dx / 2
    position.y = rect.y + rect.dy / 2
    return position
end

function GetFrame(element)
    local control = GetControl(element)
    return control:GetFrame()
end

function GetText(element)
    local control = GetControl(element)
    return autotestingSystem:GetText(control)
end

function GetElementRect(element)
    local control = GetControl(element)
    local geomData = control:GetGeometricData()
    return geomData:GetUnrotatedRect()
end

------------------------------------------------------------------------------------------------------------------------
-- Check states
------------------------------------------------------------------------------------------------------------------------
function IsVisible(element, background)
    Yield()
    local control = autotestingSystem:FindControl(element)
    return toboolean(control and control:GetVisible() and control:IsOnScreen() and IsOnScreen(control, background))
end

function IsDisabled(element)
    Yield()
    local control = GetControl(element)
    return control:GetDisabled()
end

function IsOnScreen(control, background)
    local screen = background and autotestingSystem:FindControl(background) or autotestingSystem:GetScreen()
    local geomData = control:GetGeometricData()
    local rect = geomData:GetUnrotatedRect()
    geomData = screen:GetGeometricData()
    local backRect = geomData:GetUnrotatedRect()
    return toboolean((backRect.x - rect.x <= 1) and ((rect.x + rect.dx) - (backRect.x + backRect.dx) <= 1) and (backRect.y - rect.y <= 1)
            and ((rect.y + rect.dy) - (backRect.y + backRect.dy) <= 1))
end

function IsCenterOnScreen(control, background)
    local screen = background and autotestingSystem:FindControl(background) or autotestingSystem:GetScreen()
    local center = GetCenter(control)
    local geomData = screen:GetGeometricData()
    local backRect = geomData:GetUnrotatedRect()
    return toboolean((center.x >= backRect.x) and (center.x <= backRect.x + backRect.dx) and (center.y >= backRect.y)
            and (center.y <= backRect.y + backRect.dy))
end

function CheckText(name, txt)
    Log("Check that text '" .. txt .. "' is present on control " .. name)
    local control = GetControl(name)
    return autotestingSystem:CheckText(control, txt)
end

function CheckMsgText(name, key)
    Log("Check that text with key [" .. key .. "] is present on control " .. name)
    local control = GetControl(name)
    return autotestingSystem:CheckMsgText(control, key)
end

------------------------------------------------------------------------------------------------------------------------
-- Waits
------------------------------------------------------------------------------------------------------------------------
function Wait(waitTime)
    waitTime = waitTime or DELAY
    local count, elapsedTime = 0, 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        coroutine.yield()
        count = count + 1
    end
    return count
end

function WaitUntil(time, func, ...)
    local waitTime, err = time or TIMEOUT, nil
    local elapsedTime, status = 0.0, nil
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        coroutine.yield()
        status, err = copcall(func, ...)
        if status and err then
            return true
        end
    end
    return false
end

function WaitControl(name, time)
    local waitTime, aSys = time or TIMEOUT, autotestingSystem
    local find_control_lua = function(x) return aSys:FindControl(x) or aSys:FindControlOnPopUp(x) end
    if WaitUntil(waitTime, find_control_lua, name) then
        return true
    end
    Log("WaitControl not found " .. name, "DEBUG")
    return false
end

function WaitControlDisappeared(name, time)
    local waitTime, aSys = time or TIMEOUT, autotestingSystem
    local not_find_control_lua = function(x) return not aSys:FindControl(x) and not aSys:FindControlOnPopUp(x) end
    if WaitUntil(waitTime, not_find_control_lua, name) then
        return true
    end
    Log("WaitControl still on the screen: " .. name, "DEBUG")
    return false
end

function WaitControlBecomeVisible(name, time)
    local waitTime = time or TIMEOUT
    if WaitUntil(waitTime, IsVisible, name) then
        return true
    end
    Log("WaitControl not found " .. name, "DEBUG")
    return false
end

function WaitUntilControlBecomeEnabled(name, time)
    local waitTime = time or TIMEOUT
    local is_enabled = function(x) return not IsDisabled(x) end
    if WaitUntil(waitTime, is_enabled, name) then
        return true
    end
    Log("WaitControl is disabled " .. name, "DEBUG")
    return false
end

------------------------------------------------------------------------------------------------------------------------
-- Setters
------------------------------------------------------------------------------------------------------------------------
function SetText(path, text, time)
    local waitTime = time or DELAY
    Log("SetText path=" .. path .. " text=" .. text)
    local res = autotestingSystem:SetText(path, text)
    Yield()
    Wait(waitTime)
    return res
end

function ClearField(field)
    SetText(field, "")
    ClickControl(field)
    KeyPress(2)
end

function SelectItemInList(listName, item)
    Log(string.format("Select '%s' cell in '%s' list.", item, listName))
    assert(WaitControl(listName), "Couldn't find " .. listName)

    local function __click() if IsVisible(item, listName) and IsVisible(item) then ClickControl(item) return true end return false end

    if __click() then
        return true
    end
    local listControl = GetControl(listName)
    local startPoint, __scroll = listControl:GetPivotPoint(), nil
    print(string.format('Start points X: %f; Y: %f', startPoint.x, startPoint.y))
    local finalPoint = autotestingSystem:GetMaxListOffsetSize(listControl)
    print(string.format('Final point: ' .. finalPoint))
    if autotestingSystem:IsListHorisontal(listControl) then
        __scroll, startPoint = HorizontalScroll, startPoint.y
    else
        __scroll, startPoint = VerticalScroll, startPoint.x
    end
    
    local function __getPosition() return autotestingSystem:GetListScrollPosition(listControl) end

    -- move to start of list and check cell
    local isEnd = false
    if __getPosition() == finalPoint then isEnd = true end
    for _ = 0, MAX_LIST_COUNT do
        local position = __getPosition()
        if position <= startPoint then
            break
        end
        __scroll(listName, true)
        if __click() then
            return true
        end
    end

    -- move to end of list and check cell
    if not isEnd then
        for _ = 0, MAX_LIST_COUNT do
            local position = __getPosition()
            if position >= finalPoint then
                break
            end
            __scroll(listName)
            if __click() then
                return true
            end
        end
    end
    Log(string.format("Cell '%s' in '%s' list is not found", item, listName))
    return false
end

function SelectItemInContainer(containerName, item, notInCenter, __condition)
    Log(string.format("Select '%s' cell in '%s' container.", item, containerName))
    assert(WaitControl(containerName), "Couldn't find " .. containerName)

    if not __condition then
        __condition = function(x) return true end
    end

    local function __click()
        if IsVisible(item, containerName) and IsVisible(item) and __condition(item) then
            ClickControl(item)
            return true
        end
        return false
    end

    if __click() then
        return true
    end
    local containerCtrl, position, invert = GetControl(containerName)
    local startPoint = containerCtrl:GetPivotPoint()
    print(string.format('Start points X: %f; Y: %f', startPoint.x, startPoint.y))
    local finalPoint = autotestingSystem:GetMaxContainerOffsetSize(containerCtrl)
    print(string.format('Final point: X: %f; Y: %f', finalPoint.x, finalPoint.y))

    local function __getPosition() return autotestingSystem:GetContainerScrollPosition(containerCtrl) end

    -- move to start of list and check cell
    for _ = 0, MAX_LIST_COUNT do -- move to up side
        position = __getPosition()
        if position.y <= startPoint.y then
            break
        end
        VerticalScroll(containerName, true, notInCenter)
        if __click() then
            return true
        end
    end
    -- move to left side
    for _ = 0, MAX_LIST_COUNT do
        position = __getPosition()
        if position.x <= startPoint.x then
            break
        end
        HorizontalScroll(containerName, true)
        if __click() then
            return true
        end
    end
    -- move to rigth side and down side, then to left and down
    for _ = 0, MAX_LIST_COUNT do
        invert = _ % 2 ~= 0 -- if true - right side, else - left
        -- move to right/left side
        for __ = 0, MAX_LIST_COUNT do
            position = __getPosition()
            if invert then
                if position.x <= startPoint.x then
                    break
                end
            else
                if position.x >= finalPoint.x then
                    break
                end
            end
            HorizontalScroll(containerName, invert)
            if __click() then
                return true
            end
        end
        -- move to down side
        position = __getPosition()
        if position.y >= finalPoint.y then
            break
        end
        VerticalScroll(containerName, false, notInCenter)
        if __click() then
            return true
        end
    end
    Log(string.format("Item '%s' in '%s' container is not found", item, containerName))
    return false
end

function VerticalScroll(list, invert, notInCenter)
    local position, new_position = __GetNewPosition(list, true, invert, notInCenter)
    TouchMove(position, new_position)
end

function HorizontalScroll(list, invert)
    local position, new_position = __GetNewPosition(list, false, invert)
    TouchMove(position, new_position)
end

----------------------------------------------------------------------------------------------------
-- Touch and click actions
----------------------------------------------------------------------------------------------------

-- Touch down
function TouchDownPosition(position, touchId)
    autotestingSystem:TouchDown(position, touchId or 1)
    Yield()
end

function TouchDown(x, y, touchId)
    local position = Vector.Vector2(x, y)
    TouchDownPosition(position, touchId)
end

-- Touch up
function TouchUp(touchId)
    local touchId = touchId or 1
    autotestingSystem:TouchUp(touchId)
end

function ClickPosition(position, time, touchId)
    local waitTime = time or TIMECLICK
    TouchDownPosition(position, touchId)
    Wait(waitTime)
    TouchUp(touchId)
    Wait(waitTime)
end

function Click(x, y, time, touchId)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
    local position = Vector.Vector2(x, y)
    ClickPosition(position, touchId, waitTime)
end

function KeyPress(key, control)
    if control then
        ClickControl(control)
    end
    autotestingSystem:KeyPress(key)
end

function ClickControl(name, time, touchId)
    local waitTime = time or TIMEOUT
    Log("ClickControl name=" .. name .. " touchId=" .. tostring(touchId or 1) .. " waitTime=" .. waitTime)
    if not WaitControl(name, waitTime) then
        Log("Control " .. name .. " not found.")
        return false
    end
    if IsVisible(name) and IsCenterOnScreen(name) then
        local position = GetCenter(name)
        ClickPosition(position, TIMECLICK, touchId)
        return true
    end
    Log("Control " .. name .. " is not visible.")
    return false
end

-- Move touch actions
function TouchMovePosition(position, touchId)
    autotestingSystem:TouchMove(position, touchId or 1)
    Yield()
end

function TouchMove(position, new_position, time, touchId)
    local waitTime = time or TIMECLICK
    TouchDownPosition(position, touchId)
    Wait(waitTime)
    TouchMovePosition(new_position, touchId)
    Wait(waitTime)
    Wait(waitTime)
    TouchUp(touchId)
    Wait(waitTime)
end

