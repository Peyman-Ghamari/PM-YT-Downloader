chrome.action.onClicked.addListener(async (tab) => {
  if (!tab.url || !tab.url.includes("youtube.com")) {
    console.log("Not a YouTube page.");
    return;
  }

  try {
    const cookies = await chrome.cookies.getAll({ domain: ".youtube.com" });
    
    let cookiesText = "# Netscape HTTP Cookie File\n";
    cookies.forEach(c => {
      // ۱. تبدیل دقیق ساب‌دومین به TRUE یا FALSE
      const includeSubdomains = c.domain.startsWith(".") ? "TRUE" : "FALSE";
      const secure = c.secure ? "TRUE" : "FALSE";
      
      // ۲. حل مشکل اصلی پایتون: هندل کردن کوکی‌های سشن یا منقضی شده
      let expiration = 0;
      if (c.expirationDate && !isNaN(c.expirationDate)) {
        expiration = Math.floor(c.expirationDate);
        // جلوگیری از اعداد نجومی فراتر از سال ۲۰۳۸ که پایتون را کرش میدهند
        if (expiration > 2147483647) {
          expiration = 2147483647; 
        }
      }

      // ۳. ساخت ساختار کاملاً استاندارد خط با تب
      cookiesText += `${c.domain}\t${includeSubdomains}\t${c.path}\t${secure}\t${expiration}\t${c.name}\t${c.value}\n`;
    });

    const payload = {
      url: tab.url,
      cookies: cookiesText
    };

    const response = await fetch("http://localhost:12345/download", {
      method: "POST",
      headers: { "Content-Type": "text/plain" }, 
      body: JSON.stringify(payload)
    });

    if (response.ok) {
      console.log("Successfully sent validated Netscape cookies!");
    }
  } catch (error) {
    console.error("Error occurred:", error);
  }
});