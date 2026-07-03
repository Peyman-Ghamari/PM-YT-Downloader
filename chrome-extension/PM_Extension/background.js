chrome.action.onClicked.addListener(async (tab) => {
  if (!tab.url || !tab.url.includes("youtube.com")) {
    console.log("Not a YouTube page.");
    return;
  }

  try {
    const cookies = await chrome.cookies.getAll({ domain: ".youtube.com" });
    
    let cookiesText = "# Netscape HTTP Cookie File\n";
    cookies.forEach(c => {
      // 1. Precise Conversion Of Subdomain To TRUE Or FALSE
      const includeSubdomains = c.domain.startsWith(".") ? "TRUE" : "FALSE";
      const secure = c.secure ? "TRUE" : "FALSE";
      
      // 2. Solving Python Core Issue: Handling Session Or Expired Cookies
      let expiration = 0;
      if (c.expirationDate && !isNaN(c.expirationDate)) {
        expiration = Math.floor(c.expirationDate);
        // Preventing Astronomical Numbers Beyond Year 2038 That Crash Python
        if (expiration > 2147483647) {
          expiration = 2147483647; 
        }
      }

      // 3. Creating Completely Standard Line Structure With Tabs
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
