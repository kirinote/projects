function formatJST(dateString) {
  if (!dateString) return "";
  const date = new Date(dateString);
  return date.toLocaleString("ja-JP", {
    timeZone: "Asia/Tokyo",
    year: "numeric",
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit"
  });
}

const applicationDT = formatJST($("trigger.row.Application_DT"));
const preferredDT = formatJST($("trigger.row.Preferred_DT"));

return "予約を受け付けました。<br>"
+ "受付日時 " + applicationDT + "<br><br><br>"
+ "[代表者氏名] " + $("trigger.row.Name") + "<br>"
+ "[電子メールアドレス] " + $("trigger.row.Mail") + "<br>"
+ "[携帯電話番号] " + $("trigger.row.Phone") + "<br>"
+ "[来店人数] " + $("trigger.row.Guests") + "<br>"
+ "[希望日時] " + preferredDT + "<br>"
+ "[希望連絡先] " + $("trigger.row.Contact") + "<br>"
;
