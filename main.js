const core = require("./build/Release/Core");

module.exports = (req, resp, next) => {
    core.requestInfoInput({
        "ip": req.ip
    });

    if(core.checkRequest({
        "ip": req.ip
    })) {
        next();
    }
    else {
        resp.status(403);
        resp.json({
            "err": 1000,
            "msg": "Request frequency too high"
        });
    }
}
