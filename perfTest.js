const core = require("./build/Release/Core");

function test(count, ip) {
    let startTime = Date.now();

    let trueCount = 0, falseCount = 0;

    for(let i = 0; i < count; i++) {
        core.requestInfoInput({
            "ip": ip
        });
        if(core.checkRequest({
            "ip": ip
        })) trueCount++;
        else falseCount++;
    }

    console.log(trueCount + " / " + falseCount);
    let endTime = Date.now();
    console.log(endTime - startTime);
}

test(100, "127.0.0.1");
test(1000, "127.0.0.1");
test(10000, "127.0.0.1");
test(100000, "127.0.0.1");
test(1000000, "127.0.0.1");
test(100000, "127.0.0.1");
test(10000, "127.0.0.1");
test(1000, "192.168.2.1");
test(100, "127.0.0.1");
