# MeowKit 官网浏览器更新：实际部署清单

适用地址：

- 官网：`https://meowkit.cc/pages/downloads`
- 固件仓库：`https://github.com/mingolucky/meowkit-s3-firmware`
- 安装组件：ESP Web Tools 10

## 目标

用户不再下载 ZIP、不再选择 `.bin` 文件、不再填写烧录地址。用户只需进入下载模式、点击 **Update MeowKit**、选择设备并等待完成。

## 一、在 GitHub 仓库准备网页烧录资源

在 `mingolucky/meowkit-s3-firmware` 的 `main` 分支新增：

```text
docs/
├── .nojekyll
└── firmware/
    ├── manifest.json
    ├── bootloader.bin
    ├── partitions.bin
    ├── boot_app0.bin
    └── firmware.bin
```

四个文件必须来自同一次 PlatformIO 构建：

```text
.pio/build/esp32s3box/bootloader.bin
.pio/build/esp32s3box/partitions.bin
~/.platformio/packages/framework-arduinoespressif32*/tools/partitions/boot_app0.bin
.pio/build/esp32s3box/firmware.bin
```

`docs/firmware/manifest.json`：

```json
{
  "name": "MeowKit",
  "version": "1.0.0",
  "new_install_prompt_erase": true,
  "new_install_improv_wait_time": 0,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "serialType": "cdc",
      "parts": [
        { "path": "bootloader.bin", "offset": 0 },
        { "path": "partitions.bin", "offset": 32768 },
        { "path": "boot_app0.bin", "offset": 57344 },
        { "path": "firmware.bin", "offset": 65536 }
      ]
    }
  ]
}
```

偏移量对应：

```text
0x0000  bootloader.bin
0x8000  partitions.bin
0xE000  boot_app0.bin
0x10000 firmware.bin
```

当前 GitHub 说明缺少 `boot_app0.bin`。为让“完整擦除后恢复”也能启动，网页安装包应补齐它。

## 二、启用 GitHub Pages

打开仓库：

```text
Settings → Pages
```

设置：

```text
Source: Deploy from a branch
Branch: main
Folder: /docs
```

保存并等待部署。完成后访问：

```text
https://mingolucky.github.io/meowkit-s3-firmware/firmware/manifest.json
```

必须直接看到 JSON。再逐一打开四个固件 URL，确认没有 404：

```text
https://mingolucky.github.io/meowkit-s3-firmware/firmware/bootloader.bin
https://mingolucky.github.io/meowkit-s3-firmware/firmware/partitions.bin
https://mingolucky.github.io/meowkit-s3-firmware/firmware/boot_app0.bin
https://mingolucky.github.io/meowkit-s3-firmware/firmware/firmware.bin
```

## 三、先验证 GitHub 固件包

不要先改 Shopify。创建任意临时 HTTPS 页面，放入：

```html
<script
  type="module"
  src="https://unpkg.com/esp-web-tools@10/dist/web/install-button.js?module"
></script>

<esp-web-install-button
  manifest="https://mingolucky.github.io/meowkit-s3-firmware/firmware/manifest.json"
></esp-web-install-button>
```

用桌面版 Chrome 或 Edge 真机烧录。确认正常启动后再进入下一步。

## 四、把安装器放进现有 Shopify Downloads 页面

现有 `/pages/downloads` 已经在主菜单中，不需要修改截图中的菜单。需要修改的是 Downloads 页面使用的主题模板。

### 方法 A：主题编辑器有 Custom Liquid

打开：

```text
Online Store → Themes → Customize
```

顶部页面选择器切换到：

```text
Pages → Downloads
```

在 **Firmware & APP** 下方添加 **Custom Liquid**，粘贴：

```liquid
<script
  type="module"
  src="https://unpkg.com/esp-web-tools@10/dist/web/install-button.js?module"
></script>

<section class="mk-web-flasher">
  <p class="mk-web-flasher__eyebrow">MEOWKIT FIRMWARE</p>
  <h2>Keep your MeowKit up to date.</h2>
  <p class="mk-web-flasher__lead">
    Connect MeowKit. We’ll take care of the rest.
  </p>

  <div class="mk-web-flasher__steps">
    <p><b>1</b><span>Power off MeowKit and unplug the USB cable.</span></p>
    <p><b>2</b><span>Hold BOOT and switch MeowKit on.</span></p>
    <p><b>3</b><span>Wait 2 seconds, release BOOT, then connect USB.</span></p>
  </div>

  <esp-web-install-button
    manifest="https://mingolucky.github.io/meowkit-s3-firmware/firmware/manifest.json"
  >
    <button slot="activate" class="mk-web-flasher__button" type="button">
      Update MeowKit
    </button>

    <p slot="unsupported" class="mk-web-flasher__warning">
      Open this page on a computer with Chrome or Edge.
    </p>

    <p slot="not-allowed" class="mk-web-flasher__warning">
      The installer requires a secure HTTPS connection.
    </p>
  </esp-web-install-button>

  <p class="mk-web-flasher__note">
    Keep MeowKit powered on and connected until the update is complete.
    The screen may remain black in download mode. This is normal.
  </p>

  <details class="mk-web-flasher__help">
    <summary>MeowKit doesn’t appear?</summary>
    <p>Use a USB data cable, connect directly to the computer, and repeat the three steps above.</p>
  </details>
</section>

<style>
  .mk-web-flasher {
    max-width: 760px;
    margin: 48px auto;
    padding: 56px 32px;
    border-radius: 28px;
    background: #0a0c0b;
    color: #f7f7f5;
    text-align: center;
  }
  .mk-web-flasher__eyebrow {
    margin: 0 0 16px;
    color: #bee700;
    font-size: 12px;
    font-weight: 700;
    letter-spacing: .16em;
  }
  .mk-web-flasher h2 {
    margin: 0;
    color: inherit;
    font-size: clamp(34px, 6vw, 58px);
    line-height: 1.06;
    letter-spacing: -.035em;
  }
  .mk-web-flasher__lead {
    margin: 20px 0 34px;
    color: #b9bdb8;
    font-size: 18px;
  }
  .mk-web-flasher__steps {
    max-width: 560px;
    margin: 0 auto 34px;
    padding: 18px 24px;
    border: 1px solid #292d2b;
    border-radius: 20px;
    background: #121513;
    text-align: left;
  }
  .mk-web-flasher__steps p {
    display: grid;
    grid-template-columns: 30px 1fr;
    gap: 12px;
    align-items: center;
    margin: 12px 0;
  }
  .mk-web-flasher__steps b {
    display: grid;
    width: 28px;
    height: 28px;
    place-items: center;
    border-radius: 50%;
    background: #bee700;
    color: #080a0b;
  }
  .mk-web-flasher__button {
    padding: 16px 30px;
    border: 0;
    border-radius: 999px;
    background: #bee700;
    color: #080a0b;
    font: inherit;
    font-weight: 700;
    cursor: pointer;
  }
  .mk-web-flasher__button:hover { background: #d2ff00; }
  .mk-web-flasher__warning { color: #ffcc66; }
  .mk-web-flasher__note {
    max-width: 560px;
    margin: 20px auto 0;
    color: #858b87;
    font-size: 13px;
    line-height: 1.6;
  }
  .mk-web-flasher__help {
    max-width: 560px;
    margin: 30px auto 0;
    padding-top: 20px;
    border-top: 1px solid #292d2b;
    color: #b9bdb8;
    text-align: left;
  }
  .mk-web-flasher__help summary {
    color: #fff;
    font-weight: 600;
    cursor: pointer;
  }
  @media (max-width: 749px) {
    .mk-web-flasher {
      margin: 24px 0;
      padding: 42px 20px;
      border-radius: 20px;
    }
  }
</style>
```

保存并预览。

### 方法 B：页面编辑器会删除 script

如果当前 Downloads 页面由页面构建器生成，或者 Custom Liquid 保存后按钮不出现：

1. `Online Store → Themes → … → Edit code`。
2. 新建 `sections/meowkit-web-flasher.liquid`。
3. 将方法 A 的全部代码放入该文件。
4. 在末尾添加 Shopify section schema。
5. 新建或修改 Downloads 使用的 JSON template，将该 section 加入页面。

不要把安装器放进 iframe。直接加载 Web Component，Web Serial 权限路径更短。

## 五、用户实际更新流程

官网最终只需要展示：

1. 用电脑上的 Chrome 或 Edge 打开 `meowkit.cc/pages/downloads`。
2. 关闭 MeowKit，并拔下 USB 线。
3. 按住 **BOOT**，同时开机。
4. 等待 2 秒，松开 **BOOT**，再连接 USB。
5. 点击 **Update MeowKit**。
6. 选择新出现的 `USB JTAG/serial debug unit`。
7. 选择安装，等待完成。
8. 拔下 USB，关闭设备，再正常开机。

如果安装器询问是否擦除：

- 普通更新：不要选择擦除，尽量保留内部设置。
- 固件损坏或无法启动：选择擦除，执行完整恢复。

microSD 卡内容不会被内部 Flash 烧录修改。

## 六、发布新版固件

每次发布：

1. 从同一次 PlatformIO 构建取得四个文件。
2. 更新 `manifest.json` 中的版本号。
3. 先发布到测试分支或测试目录。
4. 真机验证普通更新、擦除恢复和中断恢复。
5. 验证通过后更新 `docs/firmware/`。
6. 等待 GitHub Pages 部署完成。
7. 在官网 Downloads 页面再烧录一台设备。

Shopify 代码和 manifest URL 不需要随版本改变。

## 七、上线检查

- manifest 和四个 `.bin` URL 都返回 200
- Chrome/Edge 能看到串口选择窗口
- ESP Web Tools 识别为 ESP32-S3
- 烧录结束后 MeowKit 能正常启动
- 擦除恢复也能正常启动
- 更新中途拔线后可以重新恢复
- Safari、iPhone、iPad 以及任何未通过组件检测的浏览器显示明确的兼容提示
- 页面没有让用户选择文件或填写地址
