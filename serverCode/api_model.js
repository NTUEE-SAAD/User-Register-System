router.post("/api/fingerprint", async (req, res, next) => {
    let DB_result = {}
    try{
          const pairedMember = await member.find({fingerprint_id :req.body.fingerID})
          DB_result = pairedMember[0]
      } catch (err) {
        res.json("DB find() error in fingerprint")
      }
  
    if (DB_result.role === "admin"||DB_result.role === "mks_member") {
      let result = "";
      result = await RPiOpen(DB_result.chinese_name,"fingerprint");
      console.log("result", result);
      if (result === "ok") {
        console.log("webserver ok");
  
        res.send("ok");
      } else {
        console.log("Webserver can not open the door");
        res.send("fail");
      }
    } else {
      res.send("not authorized")
  //////other role
    }
  
  });