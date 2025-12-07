define hook-quit
    set confirm off
end
set height unlimited
target remote :61234
monitor reset
load
continue
